template<class T>
class optional {
    bool exist_data;
    T data;
public:
    optional() : exist_data(false) {}

    optional(T && t) : exist_data(true), data(t) {}
   
    T & operator *() {
        return data;
    } 

    bool isExist() {
        return exist_data;
    }
};
