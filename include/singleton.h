

template <class T>
class Singleton {
public:
    static T *Get() { return *Getpptr(); }

    template <typename... Args>
    static T *New(Args &&...args) {
        if (*Getpptr() != nullptr) {
            throw std::runtime_error("already have a Singleton instance");
        }

        T *ptr = new T(std::forward<Args>(args)...);
        *Getpptr() = ptr;
        return ptr;
    }

    static void Delete() {
        if (!*Getpptr) {
            delete Get();
            *Getpptr = nullptr;
        }
    }

private:
    static T **Getpptr() {
        static T *ptr = nullptr;
        return &ptr;
    }
};