using size_t = unsigned int;

inline void *operator new(size_t, void *p) noexcept { return p; }
inline void operator delete(void *, void *) noexcept {}

template <typename T>
class vector
{
private:
    T *arr;            
    size_t current_size; 
    size_t cap;          

    void destroy_elements()
    {
        for (size_t i = 0; i < current_size; ++i)
        {
            arr[i].~T();
        }
    }

public:
    vector() : current_size(0), cap(10)
    {
        char *raw = new char[sizeof(T) * cap];
        arr = reinterpret_cast<T *>(raw);
    }

    ~vector()
    {
        destroy_elements();
        delete[] reinterpret_cast<char *>(arr);
    }

    void push_back(const T &value)
    {
        if (current_size == cap)
        {
            size_t new_cap = cap * 2;
            char *new_raw = new char[sizeof(T) * new_cap];
            T *new_arr = reinterpret_cast<T *>(new_raw);
            for (size_t i = 0; i < current_size; ++i)
            {
                new (new_arr + i) T(arr[i]); 
            }
            destroy_elements();
            delete[] reinterpret_cast<char *>(arr);
            arr = new_arr;
            cap = new_cap;
        }
        new (arr + current_size) T(value);
        ++current_size;
    }

    void pop_back()
    {
        if (current_size > 0)
        {
            --current_size;
            arr[current_size].~T();
        }
    }

    size_t size() const
    {
        return current_size;
    }

    size_t capacity() const
    {
        return cap;
    }

    bool empty() const
    {
        return current_size == 0 ? true : false;
    }

    T &operator[](size_t index)
    {
        return arr[index];
    }

    const T &operator[](size_t index) const
    {
        return arr[index];
    }

    vector(const vector &other) : current_size(other.current_size), cap(other.cap)
    {
        char *raw = new char[sizeof(T) * cap];
        arr = reinterpret_cast<T *>(raw);
        for (size_t i = 0; i < current_size; ++i)
        {
            new (arr + i) T(other.arr[i]);
        }
    }
    vector(const char* str) {
        if (!str) return;
        
        size_t len = 0;
        const char* p = str;
        while (*p++) {
            len++;
        }
        p = str;
        for (size_t i = 0; i < len; ++i) {
            push_back((T)*p++); 
        }
    }

    vector &operator=(const vector &other)
    {
        if (this != &other)
        {
            destroy_elements();
            delete[] reinterpret_cast<char *>(arr);
            current_size = other.current_size;
            cap = other.cap;
            char *raw = new char[sizeof(T) * cap];
            arr = reinterpret_cast<T *>(raw);
            for (size_t i = 0; i < current_size; ++i)
            {
                new (arr + i) T(other.arr[i]);
            }
        }
        return *this;
    }

    vector(vector &&other) : arr(other.arr), current_size(other.current_size), cap(other.cap)
    {
        other.arr = nullptr;
        other.current_size = 0;
        other.cap = 0;
    }

    vector &operator=(vector &&other)
    {
        if (this != &other)
        {
            destroy_elements();
            delete[] reinterpret_cast<char *>(arr);
            arr = other.arr;
            current_size = other.current_size;
            cap = other.cap;
            other.arr = nullptr;
            other.current_size = 0;
            other.cap = 0;
        }
        return *this;
    }

    void clear()
    {
        destroy_elements();
        current_size = 0;
    }

    void resize(size_t n)
    {
        if (n > cap)
        {
            size_t new_cap = n > 2 * cap ? n : 2 * cap;
            char *new_raw = new char[sizeof(T) * new_cap];
            T *new_arr = reinterpret_cast<T *>(new_raw);
            for (size_t i = 0; i < current_size; ++i)
            {
                new (new_arr + i) T(arr[i]);
            }
            for (size_t i = current_size; i < n; ++i)
            {
                new (new_arr + i) T();
            }
            destroy_elements();
            delete[] reinterpret_cast<char *>(arr);
            arr = new_arr;
            cap = new_cap;
            current_size = n;
        }
        else if (n > current_size)
        {
            for (size_t i = current_size; i < n; ++i)
            {
                new (arr + i) T();
            }
            current_size = n;
        }
        else
        {
            for (size_t i = n; i < current_size; ++i)
            {
                arr[i].~T();
            }
            current_size = n;
        }
    }

    void reserve(size_t n)
    {
        if (n > cap)
        {
            char *new_raw = new char[sizeof(T) * n];
            T *new_arr = reinterpret_cast<T *>(new_raw);
            for (size_t i = 0; i < current_size; ++i)
            {
                new (new_arr + i) T(arr[i]);
            }
            destroy_elements();
            delete[] reinterpret_cast<char *>(arr);
            arr = new_arr;
            cap = n;
        }
    }

    T *begin()
    {
        return arr;
    }

    T *end()
    {
        return arr + current_size;
    }

    const T *begin() const
    {
        return arr;
    }

    const T *end() const
    {
        return arr + current_size;
    }
};