
#include "PacketBuffer.hpp"

using namespace std;
#include <assert.h>
#ifndef TAG
#define TAG "PacketBuffer"
#endif
PacketBuffer::PacketBuffer(int size)
{
    nb_bytes = size;
    currrentPos = startPos = new char[nb_bytes];
    bRemove=true;
    index=1000000;
}
PacketBuffer::PacketBuffer(PacketBuffer *clone_other)
{
    nb_bytes = clone_other->size();
    currrentPos = startPos = new char[nb_bytes];
    memcpy(startPos, clone_other->data(), nb_bytes);
    bRemove=true;
    index=1000000;
}
PacketBuffer::PacketBuffer(char* b, int nn, bool _bRemove, int _index)
{
    currrentPos = startPos = b;
    nb_bytes = nn;
    bRemove=_bRemove;
    index=_index;
    // TODO: support both little and big endian.
//    srs_assert(srs_is_little_endian());
}

PacketBuffer::~PacketBuffer()
{
    if(nb_bytes>100 && bRemove && startPos!=NULL)
    {
        free(startPos);
//        try {
//            free(bytes);
//        } catch (string msg) {
//            //cout << "memory delete error: "<<msg <<std::endl;
//        }
//        
    }

    currrentPos=startPos=NULL;
    nb_bytes=0;
}
void PacketBuffer::reset()
{
    reset_pointer();
    set_size(0);
}
char* PacketBuffer::data()
{
    return startPos;
}

char* PacketBuffer::head()
{
    return currrentPos;
}

int PacketBuffer::size()
{
    return nb_bytes;
}

void PacketBuffer::set_size(int v)
{
    nb_bytes = v;
}

int PacketBuffer::pos()
{
    return (int)(currrentPos - startPos);
}

int PacketBuffer::left()
{
    return nb_bytes - (int)(currrentPos - startPos);
}

bool PacketBuffer::empty()
{
    return !startPos || (currrentPos >= startPos + nb_bytes);
}
void PacketBuffer::reset_pointer()
{
    currrentPos=startPos;
}
bool PacketBuffer::require(int required_size)
{
    assert(required_size >= 0);
    
    return required_size <= nb_bytes - (currrentPos - startPos);
}

void PacketBuffer::skip(int size)
{
    //nb_bytes:顯示當前緩衝區長度 curpos-startpos 當前和起始的距離 size 要跳過的字節數
    printf("length %d, current %d, skip %d \n", nb_bytes, currrentPos-startPos, size);
    assert(currrentPos); //判定不為空指標
    assert(currrentPos + size >= startPos);
    assert(currrentPos + size <= startPos + nb_bytes); //要跳過的字節數不會小於緩衝區範圍
    
    currrentPos += size;
}

int8_t PacketBuffer::read_1bytes()
{
//    printf("=>%d \n", pos());
    assert(require(1));
    
    return (int8_t)*currrentPos++;
}

int16_t PacketBuffer::read_2bytes()
{
    assert(require(2));
    
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *currrentPos++;
    pp[0] = *currrentPos++;
    
    return value;
}

int16_t PacketBuffer::read_le2bytes()
{
    assert(require(2));

    int16_t value;
    char* pp = (char*)&value;
    pp[0] = *currrentPos++;
    pp[1] = *currrentPos++;

    return value;
}

int32_t PacketBuffer::read_3bytes()
{
    assert(require(3));
    
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[0] = *currrentPos++;
    
    return value;
}

int32_t PacketBuffer::read_le3bytes()
{
    assert(require(3));

    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[0] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[2] = *currrentPos++;

    return value;
}

int32_t PacketBuffer::read_4bytes()
{
    assert(require(4));
    
    int32_t value; //存放讀取的整數值
    char* pp = (char*)&value; //pp指到最前面
    pp[3] = *currrentPos++; //指標往後移動
    pp[2] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[0] = *currrentPos++;
    
    return value;
}

int32_t PacketBuffer::read_le4bytes()
{
    assert(require(4));

    int32_t value;
    char* pp = (char*)&value;
    pp[0] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[2] = *currrentPos++;
    pp[3] = *currrentPos++;

    return value;
}

int64_t PacketBuffer::read_8bytes()
{
    // 使用 assert 確保緩衝區中有足夠的數據可供讀取（至少 8 字節）。
    assert(require(8));
    
    int64_t value;
    char* pp = (char*)&value;
    pp[7] = *currrentPos++;
    pp[6] = *currrentPos++;
    pp[5] = *currrentPos++;
    pp[4] = *currrentPos++;
    pp[3] = *currrentPos++;
    pp[2] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[0] = *currrentPos++;
    
    return value;
}

int64_t PacketBuffer::read_le8bytes()
{
    assert(require(8));

    int64_t value;
    char* pp = (char*)&value;
    pp[0] = *currrentPos++;
    pp[1] = *currrentPos++;
    pp[2] = *currrentPos++;
    pp[3] = *currrentPos++;
    pp[4] = *currrentPos++;
    pp[5] = *currrentPos++;
    pp[6] = *currrentPos++;
    pp[7] = *currrentPos++;

    return value;
}
void PacketBuffer::dump()
{
    int len=pos();
    for(int i=0;i<len;i++)
    {
        printf("%02x ", (unsigned char)startPos[i]);
        if(i%16==15)
            printf("\n");
    }
    printf("\n");
}
void PacketBuffer::dumpAll()
{
    int len=size();
    for(int i=0;i<len;i++)
    {
        printf("%02x ", (unsigned char)startPos[i]);
        if(i%16==15)
            printf("\n");
    }
    printf("\n");
}
string PacketBuffer::read_string(int len)
{
    assert(require(len));
    
    std::string value;
    value.append(currrentPos, len);
    
    currrentPos += len;
    
    return value;
}

void PacketBuffer::read_bytes(char* data, int size)
{
    assert(require(size));
    
    memcpy(data, currrentPos, size);
    
    currrentPos += size;
}

void PacketBuffer::write_1bytes(int8_t value)
{
    assert(require(1)); //assert宣告 接收表達是為參數 參數若為0則終止程序執行
    
    *currrentPos++ = value; //指針往後移動
}

void PacketBuffer::write_2bytes(int16_t value)
{
    assert(require(2));
    
    char* pp = (char*)&value;
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[0];
}

void PacketBuffer::write_le2bytes(int16_t value)
{
    assert(require(2));

    char* pp = (char*)&value;
    *currrentPos++ = pp[0];
    *currrentPos++ = pp[1];
}

void PacketBuffer::write_4bytes(int32_t value)
{
    assert(require(4));
    
    char* pp = (char*)&value;
    *currrentPos++ = pp[3];
    *currrentPos++ = pp[2];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[0];
}

void PacketBuffer::write_le4bytes(int32_t value)
{
    assert(require(4));

    char* pp = (char*)&value;
    *currrentPos++ = pp[0];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[2];
    *currrentPos++ = pp[3];
}

void PacketBuffer::write_3bytes(int32_t value)
{
    assert(require(3));
    
    char* pp = (char*)&value;
    *currrentPos++ = pp[2];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[0];
}

void PacketBuffer::write_le3bytes(int32_t value)
{
    assert(require(3));

    char* pp = (char*)&value;
    *currrentPos++ = pp[0];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[2];
}

void PacketBuffer::write_8bytes(int64_t value)
{
    assert(require(8));
    
    char* pp = (char*)&value;
    *currrentPos++ = pp[7];
    *currrentPos++ = pp[6];
    *currrentPos++ = pp[5];
    *currrentPos++ = pp[4];
    *currrentPos++ = pp[3];
    *currrentPos++ = pp[2];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[0];
}

void PacketBuffer::write_le8bytes(int64_t value)
{
    assert(require(8));

    char* pp = (char*)&value;
    *currrentPos++ = pp[0];
    *currrentPos++ = pp[1];
    *currrentPos++ = pp[2];
    *currrentPos++ = pp[3];
    *currrentPos++ = pp[4];
    *currrentPos++ = pp[5];
    *currrentPos++ = pp[6];
    *currrentPos++ = pp[7];
}

void PacketBuffer::write_string(string value)
{
    assert(require((int)value.length()));
    
    memcpy(currrentPos, value.data(), value.length());
    currrentPos += value.length();
}

void PacketBuffer::write_bytes(char* data, int size)
{
    assert(require(size));
    
    memcpy(currrentPos, data, size); //把data裡的size個東東放進currentPos
    currrentPos += size;
}

