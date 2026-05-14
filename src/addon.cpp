#include <napi.h>
#include <fstream>
#include "sha2/sha512_core.h"
// Tương lai bạn include thêm ở đây: #include "sha2/sha256_core.h"

// Cấu trúc dữ liệu để gửi % tiến độ
struct ProgressData {
    double percentage;
};

// Đổi sang AsyncProgressQueueWorker
class HashWorker : public Napi::AsyncProgressQueueWorker<ProgressData> {
public:
    HashWorker(Napi::Function& callback, std::string input, bool is_file, std::string algo, Napi::Promise::Deferred deferred)
        : Napi::AsyncProgressQueueWorker<ProgressData>(callback), input(input), is_file(is_file), algo(algo), deferred(deferred) {}

    Napi::Promise GetPromise() { return deferred.Promise(); }

protected:
    void Execute(const ExecutionProgress& progress) override {
        try {
            if (is_file) {
                std::ifstream file(input, std::ios::binary | std::ios::ate);
                if (!file.is_open()) throw std::runtime_error("Không thể mở file.");
                
                // Lấy tổng dung lượng file để tính %
                std::streamsize total_size = file.tellg();
                file.seekg(0, std::ios::beg);
                
                uint8_t buffer[8192]; // Chunk 8KB
                std::streamsize bytes_read_total = 0;
                double last_reported_pct = 0;

                // Khởi tạo các cỗ máy trạng thái
                sha512::SHA512 hasher512;
                // TODO: Tạo instance cho sha256 ở đây nếu algo == "sha256"

                while (file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) {
                    std::streamsize bytes = file.gcount();
                    
                    // Routing thuật toán
                    if (algo == "sha512") hasher512.update(buffer, bytes);
                    // else if (algo == "sha256") hasher256.update(buffer, bytes);
                    
                    bytes_read_total += bytes;
                    
                    // Tính và báo cáo phần trăm (chỉ báo khi nhảy >= 1% để chống nghẽn UI)
                    if (total_size > 0) {
                        double current_pct = (static_cast<double>(bytes_read_total) / total_size) * 100.0;
                        if (current_pct - last_reported_pct >= 1.0 || current_pct == 100.0) {
                            ProgressData pd = { current_pct };
                            progress.Send(&pd, 1);
                            last_reported_pct = current_pct;
                        }
                    }
                }
                
                // Xử lý nốt chunk cuối cùng
                if (file.gcount() > 0) {
                    if (algo == "sha512") hasher512.update(buffer, file.gcount());
                }
                
                // Chốt sổ
                if (algo == "sha512") hash_result = hasher512.finalize();
                else hash_result = "Thuật toán " + algo + " chưa được cài đặt trong C++!";

            } else {
                // Băm Text siêu nhanh, bắn thẳng 100%
                sha512::SHA512 hasher512;
                if (algo == "sha512") {
                    hasher512.update(reinterpret_cast<const uint8_t*>(input.data()), input.length());
                    hash_result = hasher512.finalize();
                } else {
                    hash_result = "Thuật toán " + algo + " chưa được cài đặt!";
                }
                ProgressData pd = { 100.0 };
                progress.Send(&pd, 1);
            }
        } catch (const std::exception& e) {
            SetError(e.what());
        }
    }

    // Nhận data từ luồng nền và ném vào hàm Callback của Javascript
    void OnProgress(const ProgressData* data, size_t count) override {
        Napi::Env env = Env();
        if (count > 0) {
            Callback().Call({Napi::Number::New(env, data[0].percentage)});
        }
    }

    void OnOK() override {
        Napi::Env env = Env();
        deferred.Resolve(Napi::String::New(env, hash_result));
    }

    void OnError(const Napi::Error& e) override {
        deferred.Reject(e.Value());
    }

private:
    std::string input;
    bool is_file;
    std::string algo;
    std::string hash_result;
    Napi::Promise::Deferred deferred;
};

// Wrapper nhận 4 tham số: (input, is_file, algo, progress_callback)
Napi::Value HashAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 4 || !info[3].IsFunction()) {
        Napi::TypeError::New(env, "Tham số sai: (String, Boolean, String, Function)").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string input = info[0].As<Napi::String>().Utf8Value();
    bool is_file = info[1].As<Napi::Boolean>().Value();
    std::string algo = info[2].As<Napi::String>().Utf8Value();
    Napi::Function progress_callback = info[3].As<Napi::Function>();
    
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    HashWorker* worker = new HashWorker(progress_callback, input, is_file, algo, deferred);
    worker->Queue(); 
    return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "hash"), Napi::Function::New(env, HashAsync));
    return exports;
}

NODE_API_MODULE(sha512_addon, Init)