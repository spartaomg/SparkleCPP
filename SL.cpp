#include "common.h"

const int SL_size = 838;
unsigned char SL[SL_size] = {
0x01, 0x08, 0x0b, 0x08, 0x0a, 0x00, 0x9e, 0x32, 0x30, 0x36, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xa9, 0xff, 0x85, 0xfc, 0xa2, 0x04, 0xa9, 0x08, 0x85, 0xba, 0xa5, 0xba, 0x20, 0x0c, 0xed,
0xa9, 0x6f, 0x20, 0xb9, 0xed, 0x30, 0x3a, 0xa5, 0xba, 0x85, 0xfb, 0x20, 0xfe, 0xed, 0xe6, 0xfc,
0xf0, 0x2f, 0xad, 0x18, 0xd0, 0x30, 0xda, 0xa0, 0x03, 0xa2, 0x00, 0xa9, 0x20, 0x9d, 0x00, 0x3c,
0xe8, 0xd0, 0xfa, 0xee, 0x3e, 0x08, 0x88, 0x10, 0xf4, 0xa2, 0x4e, 0xbd, 0x0c, 0x09, 0x9d, 0xb9,
0x3d, 0xad, 0x86, 0x02, 0x9d, 0xb9, 0xd9, 0xca, 0x10, 0xf1, 0xa9, 0xf5, 0x8d, 0x18, 0xd0, 0x30,
0xb0, 0xe6, 0xba, 0xca, 0xd0, 0xb5, 0xa9, 0x15, 0x8d, 0x18, 0xd0, 0xa5, 0xfc, 0xf0, 0x12, 0xa2,
0x28, 0xbd, 0x5a, 0x09, 0x20, 0xd2, 0xff, 0xca, 0xd0, 0xf7, 0x8e, 0x01, 0x08, 0x8e, 0x02, 0x08,
0x60, 0xa2, 0x83, 0xa0, 0x09, 0xa9, 0x22, 0x20, 0xf9, 0xfd, 0xa9, 0x0f, 0xa8, 0xa6, 0xfb, 0x20,
0x00, 0xfe, 0x20, 0xc0, 0xff, 0x78, 0xa9, 0x35, 0x85, 0x01, 0xa2, 0x5f, 0x9a, 0xa9, 0x3c, 0x8d,
0x02, 0xdd, 0xa2, 0x00, 0x8e, 0x00, 0xdd, 0xbd, 0xa5, 0x09, 0x9d, 0x60, 0x01, 0xbd, 0x45, 0x0a,
0x9d, 0x00, 0x02, 0xe8, 0xd0, 0xf1, 0xad, 0x12, 0xd0, 0xcd, 0x12, 0xd0, 0xf0, 0xfb, 0x30, 0xf6,
0xc9, 0x20, 0xb0, 0x2c, 0xa9, 0x40, 0x8d, 0xaa, 0x01, 0xa9, 0xdc, 0x8d, 0xab, 0x01, 0xa2, 0x02,
0x9d, 0x9d, 0x01, 0x9d, 0xb7, 0x01, 0x9d, 0xc4, 0x01, 0xa9, 0xf8, 0xca, 0xd0, 0xf2, 0xa9, 0xb9,
0x8d, 0x9d, 0x01, 0xa9, 0x19, 0x8d, 0xa9, 0x01, 0x8d, 0xb7, 0x01, 0xa9, 0x39, 0x8d, 0xc4, 0x01,
0xa9, 0xff, 0x8d, 0xfa, 0xff, 0xa9, 0x02, 0x8d, 0xfb, 0xff, 0xa9, 0xf8, 0x2c, 0x00, 0xdd, 0x30,
0xfb, 0x8d, 0x00, 0xdd, 0xa9, 0x10, 0x48, 0xa9, 0xad, 0x48, 0x4c, 0x87, 0x01, 0x13, 0x10, 0x01,
0x12, 0x0b, 0x0c, 0x05, 0x20, 0x13, 0x15, 0x10, 0x10, 0x0f, 0x12, 0x14, 0x13, 0x20, 0x0f, 0x0e,
0x0c, 0x19, 0x20, 0x0f, 0x0e, 0x05, 0x20, 0x01, 0x03, 0x14, 0x09, 0x16, 0x05, 0x20, 0x04, 0x12,
0x09, 0x16, 0x05, 0x20, 0x10, 0x0c, 0x13, 0x20, 0x14, 0x15, 0x12, 0x0e, 0x20, 0x05, 0x16, 0x05,
0x12, 0x19, 0x14, 0x08, 0x09, 0x0e, 0x07, 0x20, 0x05, 0x0c, 0x13, 0x05, 0x20, 0x0f, 0x06, 0x06,
0x20, 0x0f, 0x0e, 0x20, 0x14, 0x08, 0x05, 0x20, 0x02, 0x15, 0x13, 0x21, 0x4e, 0x49, 0x41, 0x47,
0x41, 0x20, 0x44, 0x41, 0x4f, 0x4c, 0x20, 0x44, 0x4e, 0x41, 0x20, 0x4e, 0x4f, 0x20, 0x45, 0x56,
0x49, 0x52, 0x44, 0x20, 0x52, 0x55, 0x4f, 0x59, 0x20, 0x4e, 0x52, 0x55, 0x54, 0x20, 0x45, 0x53,
0x41, 0x45, 0x4c, 0x50, 0x4d, 0x2d, 0x45, 0x05, 0x02, 0xa2, 0x08, 0xa9, 0x12, 0xa0, 0x0f, 0x95,
0x06, 0x94, 0x07, 0x88, 0xca, 0xca, 0x10, 0xf7, 0xa9, 0x04, 0x85, 0xf9, 0x20, 0x86, 0xd5, 0xc6,
0xf9, 0x10, 0xf9, 0x4c, 0x00, 0x07, 0x85, 0x04, 0x20, 0xe5, 0x01, 0xa2, 0x18, 0x8e, 0x00, 0xdd,
0x2c, 0x00, 0xdd, 0x30, 0xfb, 0x0b, 0x31, 0x69, 0xe7, 0x8f, 0x00, 0xdd, 0x29, 0x10, 0x49, 0x10,
0x66, 0x04, 0xd0, 0xf3, 0xa9, 0xf8, 0x8d, 0x00, 0xdd, 0x60, 0x20, 0x60, 0x01, 0x20, 0xe5, 0x01,
0xa2, 0x00, 0xa0, 0x08, 0x8c, 0x00, 0xdd, 0x2c, 0x00, 0xdd, 0x70, 0xfb, 0x8e, 0x00, 0xdd, 0xca,
0x20, 0xe5, 0x01, 0xad, 0x00, 0xdd, 0x8c, 0x00, 0xdd, 0x4a, 0x4a, 0xe8, 0xea, 0xa0, 0xc0, 0x0d,
0x00, 0xdd, 0x8c, 0x00, 0xdd, 0x4a, 0x4a, 0xe0, 0x51, 0xf0, 0x1c, 0xa0, 0x08, 0x0d, 0x00, 0xdd,
0x8c, 0x00, 0xdd, 0x4a, 0x4a, 0x8d, 0xcb, 0x01, 0xa9, 0xc0, 0x2d, 0x00, 0xdd, 0x8d, 0x00, 0xdd,
0x09, 0x00, 0x9d, 0x00, 0x03, 0x50, 0xcc, 0xa0, 0x1b, 0x8c, 0xd0, 0x01, 0xd0, 0xdd, 0xd0, 0x34,
0x18, 0xca, 0xbd, 0x00, 0x03, 0xd0, 0x57, 0xca, 0x8e, 0xff, 0x03, 0xa9, 0x35, 0x85, 0x01, 0x60,
0xf0, 0x9b, 0xa9, 0xcc, 0x8d, 0xd0, 0x01, 0x8a, 0x49, 0xae, 0x8d, 0xb2, 0x01, 0x30, 0xa4, 0x20,
0x7e, 0x01, 0xa2, 0xff, 0x8e, 0xaf, 0x02, 0xe8, 0xbd, 0x00, 0x03, 0xd0, 0x05, 0xae, 0xff, 0x03,
0xd0, 0xf6, 0x85, 0x04, 0xca, 0xbd, 0x00, 0x03, 0x85, 0x02, 0xa0, 0x35, 0xca, 0xbd, 0x00, 0x03,
0xd0, 0x05, 0x88, 0xca, 0xbd, 0x00, 0x03, 0x85, 0x03, 0x84, 0x01, 0xca, 0xa0, 0x00, 0x8c, 0x00,
0x03, 0xf0, 0x7b, 0xbd, 0x00, 0x03, 0xf0, 0xb8, 0xc9, 0xf8, 0xb0, 0xa2, 0x4a, 0x4a, 0xa8, 0x49,
0xff, 0x65, 0x02, 0x85, 0x02, 0xca, 0xbd, 0x00, 0x03, 0xb0, 0x3e, 0xc6, 0x03, 0x90, 0x39, 0xbd,
0x00, 0x03, 0x2a, 0x85, 0x04, 0xca, 0xb0, 0x5c, 0x98, 0xc8, 0x8c, 0x61, 0x02, 0x49, 0xff, 0x65,
0x02, 0x85, 0x02, 0x90, 0x6b, 0x8a, 0xcb, 0x00, 0x8e, 0x66, 0x02, 0xb9, 0x00, 0x03, 0x91, 0x02,
0x88, 0xd0, 0xf8, 0xbd, 0x00, 0x03, 0x0b, 0x03, 0xf0, 0xb9, 0xa8, 0x49, 0xff, 0x65, 0x02, 0x85,
0x02, 0x90, 0x51, 0xbd, 0x00, 0x03, 0x4a, 0x4a, 0x38, 0x65, 0x02, 0x8d, 0x92, 0x02, 0xa5, 0x03,
0x69, 0x00, 0x8d, 0x93, 0x02, 0xca, 0xc8, 0xb9, 0xad, 0x10, 0x91, 0x02, 0x88, 0xd0, 0xf8, 0x06,
0x04, 0x90, 0x0b, 0xd0, 0xce, 0xbd, 0x00, 0x03, 0xca, 0x2a, 0x85, 0x04, 0xb0, 0xc5, 0x06, 0x04,
0x90, 0xa6, 0xf0, 0x9b, 0xa0, 0xf8, 0x10, 0x09, 0xbc, 0x00, 0x03, 0x98, 0xca, 0x4b, 0xf0, 0x4a,
0x4a, 0x6a, 0x8d, 0xaf, 0x02, 0x98, 0x0b, 0x0f, 0xa8, 0xd0, 0x8e, 0xbc, 0x00, 0x03, 0x90, 0x85,
0xc6, 0x03, 0x90, 0x91, 0xc6, 0x03, 0x90, 0xab, 0x8c, 0xf5, 0x02, 0x8e, 0xf6, 0x02, 0x8d, 0x12,
0xd0, 0xa9, 0xe6, 0x8d, 0xfe, 0xff, 0xa9, 0x02, 0x8d, 0xff, 0xff, 0x60, 0x48, 0x8a, 0x48, 0x98,
0x48, 0xa5, 0x01, 0x48, 0x20, 0xe5, 0x01, 0xee, 0x19, 0xd0, 0x20, 0xe9, 0x01, 0x68, 0x85, 0x01,
0x68, 0xa8, 0x68, 0xaa, 0x68, 0x40
};