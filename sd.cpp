#include "common.h"

const int sd_size = 1410;
unsigned char sd[sd_size] = {
0x18, 0xa2, 0x14, 0x94, 0x3e, 0xca, 0x10, 0xfb, 0xb0, 0x01, 0x60, 0x4c, 0x1d, 0x05, 0x68, 0xc8,
0x99, 0x00, 0x07, 0xd0, 0xf9, 0x4c, 0x17, 0x06, 0xce, 0x03, 0x37, 0xab, 0xcd, 0xef, 0x07, 0xc3,
0xa5, 0xa1, 0xa7, 0x9d, 0xad, 0xa9, 0x67, 0x04, 0x05, 0x0d, 0x77, 0x00, 0x01, 0x09, 0x47, 0xe5,
0x07, 0x0f, 0x67, 0x0a, 0x03, 0x0b, 0xa7, 0x0c, 0x06, 0x0e, 0xb7, 0x08, 0x02, 0xc3, 0x27, 0x9d,
0xaf, 0xab, 0xae, 0xaa, 0xac, 0xa8, 0xe7, 0x0a, 0x04, 0xd0, 0xf7, 0xa9, 0x08, 0x4d, 0x00, 0x1c,
0x80, 0xd1, 0x8d, 0x00, 0x1c, 0x60, 0x87, 0x86, 0x04, 0xd9, 0x97, 0xe5, 0x67, 0xc3, 0x17, 0x9d,
0xe5, 0xa3, 0xa6, 0xa2, 0xa4, 0xa0, 0xc7, 0x2f, 0x01, 0xd4, 0xd7, 0x67, 0xc3, 0x9d, 0x57, 0x2a,
0x01, 0xd5, 0xa2, 0x09, 0xcb, 0x00, 0x55, 0x30, 0x60, 0xdd, 0xa0, 0x85, 0x8f, 0x00, 0x18, 0xcc,
0x00, 0x18, 0xf0, 0xfb, 0xac, 0x00, 0x18, 0xc0, 0x80, 0x6a, 0x90, 0xf3, 0x8e, 0x00, 0x18, 0x60,
0xc3, 0xd3, 0x84, 0x21, 0x20, 0x00, 0x03, 0x90, 0x01, 0xdb, 0xa6, 0x21, 0xd6, 0x3e, 0xa7, 0x3b,
0x85, 0x20, 0xc8, 0x84, 0x08, 0x84, 0x03, 0x38, 0x80, 0xde, 0xe5, 0x00, 0xf0, 0x1c, 0x84, 0x66,
0x80, 0xd7, 0xb0, 0x0a, 0x49, 0xff, 0x69, 0x01, 0x80, 0xdf, 0xa0, 0x03, 0x84, 0x28, 0x0a, 0xa8,
0x20, 0x2d, 0x05, 0xad, 0x9c, 0x06, 0x8d, 0x00, 0x1c, 0xd8, 0xa9, 0x07, 0x85, 0x19, 0xa0, 0x47,
0x80, 0xd2, 0xa9, 0x52, 0xa2, 0x04, 0x9a, 0xa2, 0x40, 0xda, 0xd0, 0x06, 0xa0, 0x57, 0xa9, 0x55,
0xa2, 0xc0, 0x84, 0xfe, 0xa0, 0x00, 0x84, 0xa0, 0x80, 0xdc, 0x8e, 0x77, 0x04, 0xa2, 0xc0, 0xd0,
0x01, 0xd6, 0x91, 0x86, 0x91, 0x89, 0x2c, 0x00, 0x1c, 0x30, 0xfb, 0x0c, 0x01, 0x1c, 0xb8, 0x50,
0xfe, 0xcd, 0x01, 0x1c, 0xb8, 0xf0, 0x68, 0x6c, 0x18, 0x03, 0xd0, 0xc2, 0xa4, 0x19, 0xd0, 0xcc,
0x80, 0xfa, 0xc8, 0xea, 0xb1, 0x59, 0x20, 0x72, 0x03, 0xa6, 0x00, 0x85, 0x00, 0x8a, 0xc5, 0x00,
0x80, 0x7a, 0x5a, 0xda, 0xd0, 0x81, 0xb1, 0x86, 0x20, 0x72, 0x03, 0xa8, 0xb6, 0x3e, 0x10, 0x9e,
0xe8, 0x6a, 0x4a, 0xca, 0x84, 0x01, 0x30, 0xa4, 0x4c, 0x5b, 0x07, 0xd1, 0x59, 0xd0, 0xcb, 0xa0,
0x03, 0xb1, 0x10, 0xd1, 0x78, 0xd0, 0xc3, 0x88, 0xd0, 0xf7, 0xb1, 0x10, 0x85, 0x74, 0xa0, 0x05,
0x80, 0xba, 0x8a, 0xaa, 0x84, 0x56, 0xb1, 0x59, 0x91, 0x70, 0x88, 0xd0, 0xf9, 0x98, 0x4c, 0xfb,
0x05, 0x3a, 0x1a, 0x9a, 0xe0, 0x10, 0xb0, 0xd0, 0xa5, 0x63, 0x10, 0xcf, 0x6c, 0xfc, 0xff, 0x50,
0xfe, 0x2a, 0x0a, 0xad, 0x01, 0x1c, 0xcb, 0x00, 0xd0, 0x8d, 0xa2, 0x3e, 0x87, 0x98, 0x4a, 0xa6,
0x6b, 0xa5, 0x30, 0x4c, 0xe1, 0x00, 0xa6, 0x00, 0xe0, 0x12, 0xb0, 0x16, 0xa2, 0x7e, 0xd0, 0x08,
0x5d, 0x02, 0x01, 0x5d, 0x03, 0x01, 0xca, 0xca, 0x5d, 0x80, 0x01, 0x5d, 0x81, 0x01, 0xca, 0xca,
0xd0, 0xee, 0xa8, 0xd0, 0x98, 0x46, 0x19, 0xb0, 0x85, 0xa6, 0x01, 0xa5, 0x00, 0xc9, 0x12, 0xf0,
0xb3, 0x95, 0x3e, 0xc6, 0x22, 0x46, 0x76, 0x90, 0x0d, 0x4c, 0x00, 0x01, 0x68, 0xc8, 0x91, 0x7a,
0xd0, 0xfa, 0xe6, 0x29, 0xd0, 0xdd, 0xc6, 0x08, 0xc5, 0x20, 0xd0, 0x1b, 0xe4, 0x21, 0xd0, 0x17,
0xb1, 0x58, 0x85, 0x5c, 0xa9, 0x7f, 0x91, 0x58, 0x46, 0x18, 0x90, 0x07, 0x8d, 0x00, 0x01, 0xa5,
0x23, 0x91, 0x59, 0xa5, 0x08, 0xd0, 0xd5, 0xa5, 0x22, 0x05, 0x54, 0x05, 0x29, 0xd0, 0x08, 0xa5,
0x5c, 0xf0, 0x04, 0x38, 0x4c, 0x01, 0x03, 0x4c, 0xb5, 0x05, 0xe8, 0xc8, 0xa9, 0xff, 0xd0, 0x0c,
0xb5, 0x3e, 0xd0, 0xf6, 0xa9, 0xff, 0x95, 0x3e, 0x86, 0x21, 0xcb, 0x00, 0xec, 0x12, 0x05, 0x90,
0x06, 0xcb, 0x00, 0xf0, 0x02, 0xcb, 0x01, 0x88, 0xd0, 0xe6, 0x86, 0x02, 0x60, 0xa6, 0x00, 0xa0,
0x81, 0xe8, 0xe0, 0x12, 0xd0, 0x0c, 0xe8, 0xe6, 0x02, 0xe6, 0x02, 0xa0, 0x83, 0xa9, 0x98, 0x8d,
0x05, 0x1c, 0xad, 0x00, 0x1c, 0x29, 0x1b, 0x18, 0x65, 0x28, 0x09, 0x0c, 0xc0, 0x80, 0xf0, 0x11,
0x8d, 0x00, 0x1c, 0x88, 0xc0, 0x80, 0xf0, 0xed, 0x2c, 0x05, 0x1c, 0x30, 0xfb, 0xc0, 0x00, 0xd0,
0xdc, 0x86, 0x00, 0xa0, 0x11, 0xe0, 0x1f, 0xb0, 0x10, 0xc8, 0xe0, 0x19, 0xb0, 0x09, 0xc8, 0x09,
0x40, 0xe0, 0x12, 0xb0, 0x04, 0xa0, 0x15, 0x09, 0x20, 0x8c, 0x12, 0x05, 0xb6, 0x4f, 0x8e, 0x0b,
0x05, 0xa2, 0x01, 0x86, 0x28, 0xc0, 0x15, 0xf0, 0x01, 0xca, 0x8e, 0x16, 0x05, 0x46, 0x6c, 0x90,
0x01, 0x60, 0x8d, 0x9c, 0x06, 0xa9, 0xa8, 0xc0, 0x15, 0xf0, 0x02, 0xa9, 0xa2, 0x85, 0xe2, 0xa5,
0x81, 0xa6, 0x82, 0xc0, 0x13, 0xb0, 0x0a, 0xa9, 0xd0, 0xa2, 0xd3, 0xc0, 0x11, 0xd0, 0x02, 0xa2,
0xd0, 0x85, 0xaa, 0x86, 0xab, 0x46, 0x66, 0xb0, 0xd8, 0x84, 0x22, 0x46, 0x5e, 0x90, 0x06, 0x4c,
0x61, 0x06, 0x6c, 0xfc, 0xff, 0xad, 0x00, 0x1c, 0x09, 0x08, 0xaa, 0xa0, 0x64, 0xa9, 0x4f, 0x8d,
0x05, 0x1c, 0xad, 0x05, 0x1c, 0xd0, 0x0c, 0x88, 0xd0, 0xf3, 0xa9, 0x73, 0x8f, 0x00, 0x1c, 0xa9,
0x07, 0x85, 0x19, 0xad, 0x00, 0x18, 0x10, 0xda, 0x4b, 0x05, 0xb0, 0xe6, 0x8e, 0x00, 0x1c, 0xf0,
0x6a, 0xa9, 0x80, 0xa2, 0x10, 0x20, 0x7a, 0x03, 0xc9, 0xff, 0xf0, 0xc6, 0xa0, 0x00, 0x84, 0x6e,
0x84, 0x54, 0x84, 0x29, 0x0a, 0xb0, 0x4e, 0xf0, 0x02, 0xe6, 0x18, 0xa2, 0x11, 0x0a, 0x8d, 0x1a,
0x06, 0x90, 0x09, 0xe8, 0xc9, 0xf8, 0xd0, 0x04, 0xa5, 0x74, 0x85, 0x76, 0xe4, 0x56, 0xf0, 0x07,
0x86, 0x56, 0x86, 0x21, 0x4c, 0x94, 0x03, 0xa2, 0x03, 0xbd, 0x00, 0x07, 0x95, 0x20, 0xca, 0x10,
0xf8, 0x20, 0x00, 0x03, 0xe6, 0x6c, 0xaa, 0x20, 0x53, 0x05, 0xa6, 0x21, 0x98, 0x38, 0xe5, 0x22,
0xa8, 0xf0, 0x09, 0xce, 0x06, 0x05, 0x20, 0xfb, 0x04, 0xee, 0x06, 0x05, 0xc8, 0x20, 0xfb, 0x04,
0xa7, 0x20, 0x4c, 0xa2, 0x03, 0x4a, 0x85, 0x63, 0x4c, 0x92, 0x03, 0xa8, 0x46, 0x6e, 0xb0, 0xf8,
0x46, 0x54, 0x90, 0x0d, 0xa5, 0x5c, 0xf0, 0x09, 0xa5, 0x22, 0xd0, 0x05, 0xe6, 0x5e, 0x4c, 0xf3,
0x04, 0xa0, 0x00, 0xa2, 0xef, 0xa9, 0x08, 0x8d, 0x00, 0x18, 0xb9, 0x00, 0x01, 0x2c, 0x00, 0x18,
0x30, 0xfb, 0x8f, 0x00, 0x18, 0x88, 0x0a, 0x09, 0x10, 0x2c, 0x00, 0x18, 0x10, 0xfb, 0x8d, 0x00,
0x18, 0x6a, 0x4b, 0xf0, 0x2c, 0x00, 0x18, 0x30, 0xfb, 0x8d, 0x00, 0x18, 0x4a, 0x4b, 0x30, 0xc0,
0xaf, 0x2c, 0x00, 0x18, 0x10, 0xfb, 0x8d, 0x00, 0x18, 0xb0, 0xcf, 0xa9, 0x00, 0x8d, 0x00, 0x1c,
0x98, 0x49, 0xaf, 0x8d, 0x90, 0x06, 0x10, 0xc2, 0xa9, 0x10, 0x2c, 0x00, 0x18, 0x30, 0xfb, 0x8d,
0x00, 0x18, 0x2c, 0x00, 0x18, 0x10, 0xfb, 0xc8, 0x8c, 0x6c, 0x06, 0x20, 0x4b, 0x03, 0xc6, 0x03,
0xd0, 0x0d, 0xe6, 0x54, 0xa5, 0x5c, 0x85, 0x03, 0xd0, 0x05, 0xe6, 0x6e, 0x4c, 0xb5, 0x05, 0xa5,
0x08, 0xd0, 0x21, 0x46, 0x29, 0x90, 0x0a, 0xee, 0x6c, 0x06, 0xa4, 0x22, 0xd0, 0xee, 0x4c, 0xf3,
0x04, 0xa4, 0x22, 0xc4, 0x03, 0x90, 0x04, 0xa4, 0x03, 0xa6, 0x00, 0x84, 0x08, 0x86, 0x20, 0xa6,
0x02, 0x20, 0xfb, 0x04, 0x4c, 0xce, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x12, 0x00, 0x04, 0x01, 0xf0, 0x60, 0xb0, 0x20, 0x01, 0x40, 0x80, 0x00, 0xe0, 0xc0, 0xa0, 0x80,
0x07, 0x01, 0x2e, 0x1e, 0xae, 0x1f, 0xbe, 0x17, 0x00, 0x07, 0x6e, 0x1a, 0xee, 0x1b, 0xfe, 0x13,
0x01, 0x00, 0x14, 0x00, 0x8e, 0x1d, 0x9e, 0x15, 0x01, 0x00, 0x5e, 0x10, 0xce, 0x19, 0xde, 0x11,
0x7f, 0x76, 0x3e, 0x16, 0x0e, 0x1c, 0x1e, 0x14, 0x76, 0x7f, 0x7e, 0x12, 0x4e, 0x18, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
0x00, 0x00, 0x00, 0x0e, 0x00, 0x0f, 0xc5, 0x07, 0xff, 0x01, 0x01, 0x0a, 0x1e, 0x0b, 0x00, 0x03,
0xfd, 0xfd, 0xfd, 0x00, 0xfc, 0x0d, 0x00, 0x05, 0xe5, 0x67, 0xc3, 0x00, 0x00, 0x09, 0x00, 0x01,
0x5f, 0x00, 0x67, 0x06, 0x02, 0x0c, 0x00, 0x04, 0x1a, 0x03, 0x00, 0x02, 0xc1, 0x08, 0xea, 0xea,
0xea, 0x4b, 0xfc, 0xd0, 0x27, 0x5d, 0x02, 0x01, 0x5d, 0x03, 0x01, 0x85, 0x9e, 0xad, 0x01, 0x1c,
0xa2, 0x0f, 0x87, 0xae, 0x6b, 0xf0, 0xa8, 0xa5, 0x00, 0x59, 0x01, 0x03, 0x48, 0x49, 0x7f, 0x49,
0x00, 0x85, 0xa0, 0xad, 0x01, 0x1c, 0xa2, 0x03, 0x87, 0xbd, 0x4b, 0xfc, 0xaa, 0xa5, 0x00, 0x75,
0x01, 0x48, 0xaf, 0x01, 0x1c, 0x4b, 0xe0, 0xa8, 0xa9, 0x1f, 0xcb, 0x00, 0xb9, 0x00, 0x04, 0x5d,
0x1e, 0x03, 0x48, 0xa7, 0x57, 0xed, 0x01, 0x1c, 0x87, 0xdd, 0x4b, 0xf8, 0x50, 0xfe, 0xa8, 0xad,
0x01, 0x1c, 0xa2, 0x3e, 0x87, 0x98, 0x4b, 0xc1, 0xaa, 0xb9, 0x12, 0x03, 0x5d, 0x00, 0x03, 0x48,
0xba, 0xd0, 0xa2, 0x45, 0xa0, 0xaa, 0xb8, 0xad, 0x01, 0x1c, 0x50, 0xfe, 0x6b, 0xf0, 0xa8, 0x8a,
0x59, 0x01, 0x03, 0xa6, 0x98, 0x55, 0x00, 0x4d, 0x02, 0x01, 0x4d, 0x03, 0x01, 0x6c, 0x47, 0x03,
0xa2, 0x06, 0xa9, 0x12, 0xa0, 0x0e, 0x95, 0x06, 0x94, 0x07, 0x88, 0xca, 0xca, 0x10, 0xf7, 0xc6,
0xf9, 0x20, 0x86, 0xd5, 0x29, 0xfe, 0xd0, 0xf9, 0xc6, 0xf9, 0x10, 0xf5, 0x78, 0xa2, 0x00, 0xbd,
0x00, 0x06, 0x95, 0x00, 0xe8, 0xd0, 0xf8, 0xa9, 0xee, 0x8d, 0x0c, 0x1c, 0xad, 0x00, 0x1c, 0x29,
0x93, 0x09, 0x4c, 0x8d, 0x00, 0x1c, 0xa9, 0x7a, 0x8d, 0x02, 0x18, 0xa9, 0x10, 0x8d, 0x00, 0x18,
0xa9, 0x01, 0x8d, 0x0b, 0x1c, 0xa9, 0x00, 0x8d, 0x04, 0x1c, 0xa9, 0x7f, 0x8d, 0x0e, 0x18, 0x8d,
0x0e, 0x1c, 0xad, 0x0d, 0x18, 0xad, 0x0d, 0x1c, 0x4c, 0xce, 0x03, 0x68, 0xc8, 0x99, 0x00, 0x06,
0xd0, 0xf9, 0xa9, 0x0e, 0x8d, 0x39, 0x04, 0xa9, 0x03, 0x8d, 0x3a, 0x04, 0x98, 0x4c, 0xfb, 0x05,
0x53, 0x50, 0x41, 0x52, 0x4b, 0x4c, 0x45, 0x20, 0x33, 0x2e, 0x31, 0x20, 0x42, 0x59, 0x20, 0x4f,
0x4d, 0x47
};