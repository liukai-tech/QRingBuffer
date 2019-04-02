#ifndef CQRINGBUFFER_H
#define CQRINGBUFFER_H

#include <QByteArray>
#include <QList>

#ifndef QRINGBUFFER_CHUNKSIZE
#define QRINGBUFFER_CHUNKSIZE 4096
#endif
enum
{
    MaxAllocSize = (1 << (std::numeric_limits<int>::digits - 1)) - 1
};

enum
{
    MaxByteArraySize = MaxAllocSize - sizeof(std::remove_pointer<QByteArray::DataPtr>::type)
};

class CQRingBuffer
{
public:
    explicit inline CQRingBuffer(int growth = QRINGBUFFER_CHUNKSIZE) :
        head(0), tail(0), tailBuffer(0), basicBlockSize(growth), bufferSize(0) { }

    inline void setChunkSize(int size) {
        basicBlockSize = size;
    }

    inline int chunkSize() const {
        return basicBlockSize;
    }

    inline qint64 nextDataBlockSize() const {
        return (tailBuffer == 0 ? tail : buffers.first().size()) - head;
    }

    inline const char *readPointer() const {
        return bufferSize == 0 ? Q_NULLPTR : (buffers.first().constData() + head);
    }

    Q_CORE_EXPORT const char *readPointerAtPosition(qint64 pos, qint64 &length) const;
    Q_CORE_EXPORT void free(qint64 bytes);
    Q_CORE_EXPORT char *reserve(qint64 bytes);
    Q_CORE_EXPORT char *reserveFront(qint64 bytes);

    inline void truncate(qint64 pos) {
        if (pos < size())
            chop(size() - pos);
    }

    Q_CORE_EXPORT void chop(qint64 bytes);

    inline bool isEmpty() const {
        return bufferSize == 0;
    }

    inline int getChar() {
        if (isEmpty())
            return -1;
        char c = *readPointer();
        free(1);
        return int(uchar(c));
    }

    inline void putChar(char c) {
        char *ptr = reserve(1);
        *ptr = c;
    }

    void ungetChar(char c)
    {
        if (head > 0) {
            --head;
            buffers.first()[head] = c;
            ++bufferSize;
        } else {
            char *ptr = reserveFront(1);
            *ptr = c;
        }
    }


    inline qint64 size() const {
        return bufferSize;
    }

    Q_CORE_EXPORT void clear();
    inline qint64 indexOf(char c) const { return indexOf(c, size()); }
    Q_CORE_EXPORT qint64 indexOf(char c, qint64 maxLength, qint64 pos = 0) const;
    Q_CORE_EXPORT qint64 read(char *data, qint64 maxLength);
    Q_CORE_EXPORT QByteArray read();
    Q_CORE_EXPORT qint64 peek(char *data, qint64 maxLength, qint64 pos = 0) const;
    Q_CORE_EXPORT void append(const char *data, qint64 size);
    Q_CORE_EXPORT void append(const QByteArray &qba);

    inline qint64 skip(qint64 length) {
        qint64 bytesToSkip = qMin(length, bufferSize);

        free(bytesToSkip);
        return bytesToSkip;
    }

    Q_CORE_EXPORT qint64 readLine(char *data, qint64 maxLength);

    inline bool canReadLine() const {
        return indexOf('\n') >= 0;
    }

private:
    QList<QByteArray> buffers;
    int head, tail;
    int tailBuffer; // always buffers.size() - 1
    int basicBlockSize;
    qint64 bufferSize;
};

#endif // QRINGBUFFER_P_H
