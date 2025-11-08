#ifndef RUNTIMEERROR
#define RUNTIMEERROR

#include <exception>
#include <string>

class RuntimeError :public std::exception {
    private:
        std::string s;
    public:
        RuntimeError(std::string);
        std::string message() const;
        //const char* what() const noexcept override;
};

#endif