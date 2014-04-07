#include <phpcpp.h>
#include <vector>

using namespace std;

class SplBitArray : public Php::Base
{
    private:
        int _size = 0;
        vector<int> _bits;

    public:
        SplBitArray(){}
        virtual ~SplBitArray(){}

        void __construct(Php::Parameters &params)
        {
            if (params.size() > 0) {
                _size = params[0];
            }

            _bits.assign((_size + 31) >> 3, 0);
        }

        Php::Value getSize()
        {
            return _size;
        }

        Php::Value getSizeInBytes()
        {
            return (_size + 7) >> 3;
        }

        void ensureCapacity(Php::Parameters &params)
        {
            int size = params[0];

            if ((unsigned)size > _bits.size() << 5) {
                _bits.resize((size + 31) >> 5, 0);
            }
        }

        Php::Value get(Php::Parameters &params)
        {
            int index = params[0];

            return (_bits[index >> 5] & (1 << (index & 0x1f))) != 0;
        }

        void set(Php::Parameters &params)
        {
            int index = params[0];

            _bits[index >> 5] = _bits[index >> 5] | 1 << (index & 0x1f);
        }

        void flip(Php::Parameters &params)
        {
            int index = params[0];

            _bits[index >> 5] ^= 1 << (index & 0x1f);
        }

        void clear()
        {
            for (unsigned i = 0; i < _bits.size(); i++) {
                _bits[i] = 0;
            }
        }
};

extern "C" {
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension splBitArrayExtension("splbitarray", "1.0");
        
        Php::Class<SplBitArray> splBitArray("SplBitArray");
        splBitArray.method("__construct", &SplBitArray::__construct, { Php::ByVal("size", Php::Type::Numeric, false) });
        splBitArray.method("getSize", &SplBitArray::getSize);
        splBitArray.method("getSizeInBytes", &SplBitArray::getSizeInBytes);
        splBitArray.method("ensureCapacity", &SplBitArray::ensureCapacity, { Php::ByVal("size", Php::Type::Numeric, false) });
        splBitArray.method("get", &SplBitArray::get, { Php::ByVal("index", Php::Type::Numeric, false) });
        splBitArray.method("set", &SplBitArray::set, { Php::ByVal("index", Php::Type::Numeric, false) });
        splBitArray.method("flip", &SplBitArray::flip, { Php::ByVal("index", Php::Type::Numeric, false) });
        splBitArray.method("clear", &SplBitArray::clear);

        splBitArrayExtension.add(std::move(splBitArray));
        
        return splBitArrayExtension;
    }
}

