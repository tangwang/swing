#include <iostream>
#include <vector>

using namespace std;

class BitMap
{
public:
    BitMap(size_t num)
    {
        _v.resize((num >> 5) + 1);
    }

    void Set(size_t num) //set 1
    {
        size_t index = num >> 5;
        size_t pos = num & 0x1F;
        _v[index] |= (1 << pos);
    }

    void Reset(size_t num) //set 0
    {
        size_t index = num >> 5;
        size_t pos = num & 0x1F;
        _v[index] &= ~(1 << pos);
    }

    // 
    void ResetRoughly(size_t num) //set 0
    {
        size_t index = num >> 5;
        _v[index] = 0;
    }

    bool Existed(size_t num)//check whether it exists
    {
        size_t index = num >> 5;
        size_t pos = num & 0x1F;
        return (_v[index] & (1 << pos));
    }

private:
    vector<size_t> _v;
}; 

