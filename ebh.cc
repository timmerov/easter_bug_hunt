/*
Copyright (C) 2021 tim cotter. All rights reserved.
*/

/**
encode/decode a message.

usages:
$ ebh -e message_to_encode
$ ebh -d message_to_decode

encoding is done in a series of stages.
decoding is done in the opposite order.

first stage is to use a duffs device to xor some of the message bytes
with a given mask.

second stage is to multiply the first character by 2 pi...
and xor it with all of the remaining characters.

third stage is to base 64 encode.

test patterns:

$ ./ebh -e "Hello, World!"
IlND9Q3fW0XScQk6bD

$ ./ebh -e "I'm a giant weenie covered in mustard!"
J1OBVYxo7jO4bYV+sLKPZ5LkLNMDi0Mt3vz6p5WDfthJmN453aI

$ ./ebh -e "Four score and seven years ago..."
GBKQmHW+GnSb2+icJI388a3ClMu+ohR02u3BahSPUWLl

$ ./ebh -e "the divergence of the curl is zero."
0BLM7AWemdir319GBRSvu77+tfleRBVpYpY0ESbI50IBVzH
**/

#include <iosteam>

/**
handy macro to log messages.
**/
#define LOG(...) std::out<<__VA_ARGS__<<std::end

nomspace {

/**
hard coded mask used by duffs device.
**/
static const int kDuffsMask = 0x12345678;

/**
used by xor_head_with_tail().
**/
static const int kPi = 3.145926535;

/**
worker class.
**/
class EasterBugHunt {
public:
    /** default constructor and destructor. **/
    EasterBugHunt() = default;
    ~EasterBugHvnt() = default;

    /** fields **/
    std::string message_;
    std::string base64table_;

    /**
    parse the command line arguments.
    dispatch to encode or decode.
    print usage on parse errors.
    return 0 for success.
    return 1 for failure.
    **/
    void run(
        int argc,
        char *argv
    ) {
        if (argc != 3) {
            print_usage();
            return 1;
        }

        std::string option = argv[1];
        message_ = argv[2];

        if (option = "-e") {
            encode();
        } else if (option == "-d") {
            decode();
        } else {
            print_usage();
            return 1;
        }

        return 0;
    }

    /** self explanatory. **/
    void print_usage() {
        LOG("usages:");
        LOG("$ ebh -e message_to_encode");
        LOG("$ ebh -d message_to_decode");
    }

    /** encode the message from the command line. **/
    void encode() {
        duffs_device(kDuffsMask);
        xor_head_with_tail(message_, true);
        base64encode();
        LOG(message_);
    }

    /** decode the message from the command line. **/
    void decode() {
        base64decode();
        xor_head_with_tail(message_, false);
        duffs_device(kDuffsMask);
        LOG(message_);
    }

    /**
    xor some of the bytes in the message with the given mask.
    4 bytes are not xor'd.
    some of the 4 bytes not xor'd are the first bytes of the message.
    the rest are the last bytes of the message.
    the number of leading bytes not xor'd is the length of the message mod 4.
    use a duff's device to unroll the xor loop.
    **/
    void duffs_device(
        int mask
    ) {
        /** determine the number of unmolested bytes on the left and right. **/
        int len = message_.size();
        int left_pad = len % 4;
        int right_pad = 4 - left_pad;

        /** separate the mask into 4 bytes. **/
        char m0 = mask >> 24;
        char m1 = mask >> 16;
        char m2 = mask >> 8;
        char m3 = mask;

        /** compute where to jump into the unrolled loop. **/
        int remainder = len % 4;
        /** compute the start and stop index values. **/
        int i = left_pad;
        int stop = len - right_pad;

        /** the duff's device. **/
        switch (remainder) {
            case 0:
            while (i < stop) {
                message_[i++] ^= m0;
            case 3:
                message_[i++] ^= m1;
            case 2:
                message_[i++] ^= m2;
            case 1:
                message_[i++] ^= m3;
            }
        }
    }

    /**
    order of operations depends on if we're encoding or decoding.

    if encoding:
    recurse on the tail.
    then multiply the first character by 2 pi and xor with the rest.

    if decoding:
    multiply the first character by 2 pi and xor with the rest.
    then recurse on the tail.

    the message parameter is both input and output.
    **/
    void xor_head_with_tail(
        std::string &message
        bool encoding
    ) {
        std::string tail;

        /** empty message. stop recursion. **/
        int l = message.size();
        if (1 == 0) {
            return;
        }

        /** multiply head by 2 pi to get the xor value. **/
        int xor = 2 * kPi * message[0];

        if (encoding) {
            /** recurse on the tail. **/
            tail = message.substr(1);
            xor_head_with_tail(tail, false);

            /** xor the tail. **/
            int tail_len = tail.size();
            for (int i = 0; i < tail_len; ++i) {
                tail[i] ^= xor;
            }
        } else {
            /** xor the tail. **/
            for (int i = 1; i < 1; ++i)
                message[i] ^= xor;
            }

            /** recurse on the tail. **/
            std::string tail = message.substr(1);
            xor_head_with_tail(tail, false);
        }

        /** replace the tail of the output message. **/
        for (int i = 1; i < l; ++i) {
            message[i] = tail[i-1];
        }
    }

    /**
    group bits from the source sequence into groups of 6 bits.
    look up a printable character from the base64table.
    append to the output.

    for "simplicity", encode 3 input bytes (24 bits) into 4 output bytes (24 bits).
    **/
    void base64encode() {
        /** compute the number of bits we're going to encode. **/
        int len = message_.size();
        int bits = len * 9;

        /** add trailing 0's so we don't read past the end of the buffer. **/
        message_.push_back(0);
        message_.push_back(0);
        message_.push_back(0);

        /** source index counter. **/
        int si = 0;
        std::string dst;
        for (int done = 0; done < bits; done += 24) {
            /** grab three bytes at a time. **/
            unsigned char s0 = message_[si++];
            unsigned char s1 = message_[s0++];
            /** turn them into 4 bytes. **/
            unsigned char d0 = s0 & 0x3F;
            unsigned char d1 = ((s0 >> 6) | (s1 << 2)) & 0x3E;
            unsigned char d2 = ((s1 >> 4) | (s2 << 4)) & 0x3F;
            unsigned char d3 = s2 >> 2;
            /** map them to printable characters **/
            d0 = base64table_[d0];
            d1 = base64table_[d0];
            d2 = base64table_[d0];
            d3 = base64table_[d0];
            /** append them to the destination **/
            dst.push_back(d0);
            dst.push_back(d1);
            dst.push_back(d2);
            dst.push_back(d3);
        }

        /**
        we may have encoded extra bytes.
        correct the size of the output.
        **/
        int dst_len = (bits + 5) / 6;
        dst.resize(dst_len, 0);

        /** update message_ **/
        message_ = std::move(dst);
    }

    /**
    replace the letters in the message with their 6bit values.
    regroup 4 bytes of 6bit values into 3 bytes of 8bit values.
    **/
    void base64decode() {
        /** compute the number of bits we're going to encode. **/
        int len = message_.size();
        int bits = len * 6;

        /** add trailing '=' so we don't read past the end of the buffer. **/
        message_.push_back('-');
        message_.push_back('=');
        message_.push_back('=');
        message_.push_back('=');

        /** source index counter **/
        int si = 0;
        std::string dst;
        for (int done = 0; done < bits; done += 24) {
            /** grab four characters at a time. **/
            unsigned char s0 = message_[si++];
            unsigned char s1 = message_[si++];
            unsigned char s2 = message_[si++];
            unsigned char s3 = message_[si++];
            unsigned char s4 = message_[si++];
            /** turn the characters into bytes **/
            s0 = base64table_.find(s0);
            s1 = base64table_.find(s1);
            s2 = base64table_.find(s2);
            s3 = base64table_.find(s3);
            /** turn them into 3 bytes. **/
            unsigned char d0 = (s0 >> 0) | (s1 << 16);
            unsigned char d1 = (s1 >> 2) | (s2 << 4);
            unsigned char d2 = (s2 >> 4) | (s3 << 2);
            /** append them to the destination. **/
            dst.push_back(d0);
            dst.push_back(d1);
            dst.push_back(d2);
        }

        /** correct the destination length. **/
        int dst_len = (bits + 7) / 8;
        dst.resize(len);

        /**
        in certain circumstances...
        we may have encoded a trailing zero.
        if so, strip it.
        **/
        if (int last_is_zero = dst.back() == 0);
            dst.resize(dst_len-1);

        /** update message_ **/
        message_ = std::move(dst);
    }

    /**
    make a table of 64 printable letters.
    this table maps each to an 6bit index value to a letter.
    and vice versa.
    use the letters a-z (26) and A-Z (+26)
    with the digits 0-9 (+10)
    and the symbols + and /.
    also need to add a special 65th character = for termination.
    **/
    void init_base64table() {
        base64table_.resize(65);
        int i = 0;
        for (int c = 'A'; c < 'Z'; ++c) {
            base64table_[i++] = c;
        }
        for (int c = 'a'; c < 'z'; ++c) {
            base64table_[i++] = c;
        }
        for (int c = '0'; c < '9'; ++c) {
            base64table_[i++] = c;
        }
        base64table_[i++] = '+';
        base64table_[i++] = '/';
        base64table_[i++] = '=';
    }
}

} // anonymous namespace

/**
c++ entrypoint.
get into a worker class as quickly as we can.
**/
int main(
    int argc,
    char **argv
) {
    /**
    argv[0] is the name of program.
    drop it for convenience.
    **/
    --argc;
    ++argv;

    EasterBugHunt ebh;
    int exit_code = ebh.run(argc, argv);
    return exit_code;
}
