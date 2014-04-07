#include <phpcpp.h>
#include <vector>

using namespace std;

class SplBitArray : public Php::Base
{
    private:
        int _size = 0;
        vector<uint32_t> _bits;

        unsigned _numberOfTrailingZeros(int value)
        {
            unsigned char count = 32;

            value &= -signed(value);
            if (value) count--;
            if (value & 0x0000ffff) count -= 16;
            if (value & 0x00ff00ff) count -= 8;
            if (value & 0x0f0f0f0f) count -= 4;
            if (value & 0x33333333) count -= 2;
            if (value & 0x55555555) count -= 1;

            return count;
        }

    public:
        SplBitArray(){}
        virtual ~SplBitArray(){}

        void __construct(Php::Parameters &params)
        {
            if (params.size() > 0) {
                _size = params[0];
            }

            _bits.assign((_size + 31) / 32, 0);
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
            ensureCapacity(int(params[0]));
        }

        void ensureCapacity(int size)
        {
            if ((unsigned)size > _bits.size() * 32) {
                _bits.resize((size + 31) * 32, 0);
            }
        }

        Php::Value get(Php::Parameters &params)
        {
            return get(int(params[0]));
        }

        bool get(int index)
        {
            return (_bits[index / 32] & (1 << (index & 0x1f))) != 0;
        }

        void set(Php::Parameters &params)
        {
            int index = params[0];

            _bits[index / 32] |= 1 << (index & 0x1f);
        }

        void flip(Php::Parameters &params)
        {
            int index = params[0];

            _bits[index / 32] ^= 1 << (index & 0x1f);
        }

        Php::Value getNextSet(Php::Parameters &params)
        {
            int from = params[0];

            if (from >= _size) {
                return _size;
            }

            int bitsOffset  = from / 32;
            int currentBits = _bits[bitsOffset];
            int bitsLength  = _bits.size();

            currentBits &= ~((1 << (from & 0x1f)) - 1);

            while (currentBits == 0) {
                if (++bitsOffset == bitsLength) {
                    return _size;
                }

                currentBits = _bits[bitsOffset];
            }

            int result = (bitsOffset * 32) + _numberOfTrailingZeros(currentBits);

            return result > _size ? _size : result;
        }

        Php::Value getNextUnset(Php::Parameters &params)
        {
            int from = params[0];

            if (from >= _size) {
                return _size;
            }

            int bitsOffset  = from / 32;
            int currentBits = _bits[bitsOffset];
            int bitsLength  = _bits.size();

            currentBits &= ~((1 << (from & 0x1f)) - 1);

            while (currentBits == 0) {
                if (++bitsOffset == bitsLength) {
                    return _size;
                }

                currentBits = ~_bits[bitsOffset];
            }

            int result = (bitsOffset * 32) + _numberOfTrailingZeros(currentBits);

            return result > _size ? _size : result;
        }

        void setBulk(Php::Parameters &params)
        {
            int index   = params[0];
            int newBits = params[1];

            _bits[index / 32] = newBits;
        }

        void setRange(Php::Parameters &params)
        {
            int start = params[0];
            int end   = params[1];

            if (start > end) {
                throw Php::Exception("'end' must be greater or equal to 'start'");
            }

            if (end == start) {
                return;
            }

            end--;

            int firstInt = start / 32;
            int lastInt  = end / 32;

            for (int i = firstInt; i <= lastInt; i++) {
                int firstBit = i > firstInt ? 0 : start & 0x1f;
                int lastBit  = i < lastInt ? 31 : end & 0x1f;
                uint32_t mask;

                if (firstBit == 0 && lastBit == 31) {
                    mask = 0x7fffffff;
                } else {
                    mask = 0;

                    for (int j = firstBit; j < lastBit; j++) {
                        mask |= 1 << j;
                    }
                }

                _bits[i] = _bits[i] | mask;
            }
        }

        Php::Value isRange(Php::Parameters &params)
        {
            int start  = params[0];
            int end    = params[1];
            bool value = params[2];

            if (start > end) {
                throw Php::Exception("'end' must be greater or equal to 'start'");
            }

            if (end == start) {
                return true;
            }

            end--;

            int firstInt = start / 32;
            int lastInt  = end / 32;

            for (int i = firstInt; i <= lastInt; i++) {
                int firstBit = i > firstInt ? 0 : start & 0x1f;
                int lastBit  = i < lastInt ? 31 : end & 0x1f;
                uint32_t mask;

                if (firstBit == 0 && lastBit == 31) {
                    mask = 0x7fffffff;
                } else {
                    mask = 0;

                    for (int j = firstBit; j < lastBit; j++) {
                        mask |= 1 << j;
                    }
                }

                if ((_bits[i] & mask) != (value ? mask : 0)) {
                    return false;
                }
            }

            return true;
        }

        void appendBit(Php::Parameters &params)
        {
            appendBit(bool(params[0]));
        }

        void appendBit(bool bit)
        {
            ensureCapacity(_size + 1);

            if (bit) {
                _bits[_size / 32] |= (1 << (_size & 0x1f));
            }

            _size++;
        }

        void appendBits(Php::Parameters &params)
        {
            bool value   = params[0];
            int  numBits = params[1];

            if (numBits < 0 || numBits > 32) {
                throw Php::Exception("'numBits' must be between 0 and 32");
            }

            ensureCapacity(_size + numBits);

            for (int numBitsLeft = numBits; numBitsLeft > 0; numBitsLeft--) {
                appendBit(((value > (numBitsLeft - 1)) & 0x01) == 1);
            }
        }

        void appendBitArray(Php::Parameters &params)
        {
            Php::Value bitArray = params[0];

            int otherSize = bitArray.call("getSize");
            ensureCapacity(_size + otherSize);

            for (int i = 0; i < otherSize; i++) {
                appendBit(bitArray.call("get", i));
            }
        }

        void xorBits(Php::Parameters &params)
        {
            Php::Value bitArray = params[0];

            int bitsLength = _bits.size();
            vector<int> otherBits = bitArray.call("getBitArray");

            if (bitsLength != int(otherBits.size())) {
                throw Php::Exception("Sizes don't match'");
            }

            for (int i = 0; i < bitsLength; i++) {
                _bits[i] ^= otherBits[i];
            }
        }

        Php::Value toBytes(Php::Parameters &params)
        {
            int bitOffset = params[0];
            int numBytes  = params[1];

            vector<unsigned char> bytes(numBytes);

            for (int i = 0; i < numBytes; i++) {
                unsigned char byte = 0;

                for (int j = 0; j < 8; j++) {
                    if (get(bitOffset)) {
                        byte |= 1 << (7 - j);
                    }

                    bitOffset++;
                }

                bytes[i] = byte;
            }

            return bytes;
        }

        Php::Value getBitArray()
        {
            return vector<int64_t>(_bits.begin(), _bits.end());
        }

        void reverse(Php::Parameters &params)
        {
            vector<uint32_t> newBits(_bits.size()); 

            int length        = ((_size - 1) / 32);
            int oldBitsLength = length + 1;

            for (int i = 0; i < oldBitsLength; i++) {
                long x = _bits[i];

                x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
                x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
                x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
                x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
                x = ((x >> 16) & 0x0000ffff) | ((x & 0x0000ffff) << 16);

                newBits[length - i] = x;
            }

            if (_size != oldBitsLength * 32) {
                int leftOffset = oldBitsLength * 32 - _size;
                int mask      = 1;

                for (int i = 0; i < 31 - leftOffset; i++) {
                    mask = (mask << 1) | 1;
                }

                int currentInt = (newBits[0] >> leftOffset) & mask;

                for (int i = 1; i < oldBitsLength; i++) {
                    int nextInt = newBits[i];
                    currentInt |= nextInt << (32 - leftOffset);
                    newBits[i - 1] = currentInt;
                    currentInt = (nextInt >> leftOffset) & mask;
                }

                newBits[oldBitsLength - 1] = currentInt;
            }

            _bits = newBits;
        }

        void clear()
        {
            for (unsigned i = 0; i < _bits.size(); i++) {
                _bits[i] = 0;
            }
        }

        Php::Value __toString()
        {
            std::string result;

            for (int i = 0; i < _size; i++) {
                if ((i & 0x07) == 0) {
                    result.append(" ");
                }

                result.append(get(i) ? "X" : ".");
            }

            return result; 
        }
};

extern "C" {
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension splBitArrayExtension("splbitarray", "1.0");
        
        Php::Class<SplBitArray> splBitArray("SplBitArray");
        splBitArray.method("__construct", &SplBitArray::__construct, {
            Php::ByVal("size", Php::Type::Numeric, false)
        });
        splBitArray.method("getSize", &SplBitArray::getSize);
        splBitArray.method("getSizeInBytes", &SplBitArray::getSizeInBytes);
        splBitArray.method("ensureCapacity", &SplBitArray::ensureCapacity, {
            Php::ByVal("size", Php::Type::Numeric, true)
        });
        splBitArray.method("get", &SplBitArray::get, {
            Php::ByVal("index", Php::Type::Numeric, true)
        });
        splBitArray.method("set", &SplBitArray::set, {
            Php::ByVal("index", Php::Type::Numeric, true)
        });
        splBitArray.method("flip", &SplBitArray::flip, {
            Php::ByVal("index", Php::Type::Numeric, true)
        });
        splBitArray.method("getNextSet", &SplBitArray::getNextSet, {
            Php::ByVal("from", Php::Type::Numeric, true)
        });
        splBitArray.method("getNextUnset", &SplBitArray::getNextUnset, {
            Php::ByVal("from", Php::Type::Numeric, true)
        });
        splBitArray.method("setBulk", &SplBitArray::setBulk, {
            Php::ByVal("index", Php::Type::Numeric, true),
            Php::ByVal("newBits", Php::Type::Numeric, true)
        });
        splBitArray.method("setRange", &SplBitArray::setRange, {
            Php::ByVal("start", Php::Type::Numeric, true),
            Php::ByVal("end", Php::Type::Numeric, true)
        });
        splBitArray.method("isRange", &SplBitArray::isRange, {
            Php::ByVal("start", Php::Type::Numeric, true),
            Php::ByVal("end", Php::Type::Numeric, true),
            Php::ByVal("value", Php::Type::Bool, true)
        });
        splBitArray.method("appendBit", &SplBitArray::appendBit, {
            Php::ByVal("bit", Php::Type::Bool, true)
        });
        splBitArray.method("appendBits", &SplBitArray::appendBits, {
            Php::ByVal("value", Php::Type::Bool, true),
            Php::ByVal("numBits", Php::Type::Numeric, true)
        });
        splBitArray.method("appendBitArray", &SplBitArray::appendBitArray, {
            Php::ByVal("bitArray", "SplBitArray", true),
        });
        splBitArray.method("xorBits", &SplBitArray::xorBits, {
            Php::ByVal("bitArray", "SplBitArray", true),
        });
        splBitArray.method("toBytes", &SplBitArray::toBytes, {
            Php::ByVal("bitOffset", Php::Type::Numeric, true),
            Php::ByVal("numBytes", Php::Type::Numeric, true)
        });
        splBitArray.method("getBitArray", &SplBitArray::getBitArray);
        splBitArray.method("reverse", &SplBitArray::reverse);
        splBitArray.method("clear", &SplBitArray::clear);

        splBitArrayExtension.add(std::move(splBitArray));
        
        return splBitArrayExtension;
    }
}

