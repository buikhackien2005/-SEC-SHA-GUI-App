#include <napi.h>
#include "utils/padding_parsing.h"
#include "sha2/sha512_core.h"

// Tạo một lớp kế thừa từ AsyncWorker
class HashWorker : public Napi::AsyncWorker {
public:
    // Constructor: Nhận dữ liệu đầu vào và tạo một đối tượng Promise
    HashWorker(Napi::Env& env, std::string input_text)
        : Napi::AsyncWorker(env), input_text(input_text), deferred(Napi::Promise::Deferred::New(env)) {}

    ~HashWorker() {}

    // Lấy đối tượng Promise để trả về cho Javascript
    Napi::Promise GetPromise() { return deferred.Promise(); }

protected:
    // ---------------------------------------------------------
    // LUỒNG NỀN (BACKGROUND THREAD) - CẤM GỌI NAPI Ở ĐÂY
    // ---------------------------------------------------------
    void Execute() override {
        try {
            std::vector<uint8_t> padded_data = pad_and_parse(input_text);
            hash_result = sha512::hash_padded_data(padded_data);
        } catch (const std::exception& e) {
            SetError(e.what()); // Bắt lỗi để truyền sang OnError
        }
    }

    // ---------------------------------------------------------
    // LUỒNG CHÍNH (MAIN THREAD) - GIAO TIẾP VỚI JAVASCRIPT
    // ---------------------------------------------------------
    void OnOK() override {
        Napi::Env env = Env();
        // C++ tính toán xong, Resolve Promise trả về chuỗi Hex
        deferred.Resolve(Napi::String::New(env, hash_result));
    }

    void OnError(const Napi::Error& e) override {
        // Nếu Execute() có lỗi, Reject Promise
        deferred.Reject(e.Value());
    }

private:
    std::string input_text;
    std::string hash_result;
    Napi::Promise::Deferred deferred;
};

// Hàm Wrapper bộc lộ ra ngoài
Napi::Value HashSHA512Async(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string input_text = info[0].As<Napi::String>().Utf8Value();
    
    // Khởi tạo Worker và ném nó vào hàng đợi (Thread Pool)
    HashWorker* worker = new HashWorker(env, input_text);
    worker->Queue(); 
    
    // Trả về một lời hứa (Promise) ngay lập tức, không chặn giao diện
    return worker->GetPromise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "hash"), Napi::Function::New(env, HashSHA512Async));
    return exports;
}

NODE_API_MODULE(sha512_addon, Init)