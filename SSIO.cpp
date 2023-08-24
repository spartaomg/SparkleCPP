#include "common.h"

const int SSIO_size = 485;
unsigned char SSIO[SSIO_size] = {
0x00, 0x77, 0xc9, 0x00, 0x90, 0x02, 0xa9, 0x00, 0x8d, 0x01, 0x03, 0xaa, 0xbd, 0xbb, 0x03, 0x8d,
0x92, 0x03, 0xa9, 0x00, 0x8d, 0xcf, 0x03, 0x8a, 0x18, 0x69, 0x00, 0x8d, 0xcd, 0x03, 0x20, 0xeb,
0x01, 0xd0, 0x78, 0xa0, 0x07, 0xb9, 0xcb, 0x03, 0x20, 0xb4, 0x03, 0x88, 0x10, 0xf7, 0xad, 0xcf,
0x03, 0x18, 0xed, 0xcb, 0x03, 0x8d, 0xcf, 0x03, 0x85, 0x02, 0xad, 0xcd, 0x03, 0xe9, 0x00, 0x8d,
0xcd, 0x03, 0x85, 0x03, 0xad, 0x00, 0x03, 0x18, 0xed, 0xcb, 0x03, 0x8d, 0x00, 0x03, 0xb0, 0x03,
0xce, 0x01, 0x03, 0xac, 0xcb, 0x03, 0xc8, 0xc6, 0x01, 0xb1, 0x02, 0xe6, 0x01, 0x20, 0xb4, 0x03,
0x88, 0xd0, 0xf4, 0xad, 0x24, 0x03, 0xc9, 0x05, 0xd0, 0x1b, 0x38, 0x6d, 0xcb, 0x03, 0x49, 0xff,
0xf0, 0x13, 0xa8, 0x88, 0xa9, 0x84, 0xd0, 0x02, 0xa9, 0x00, 0x20, 0xb4, 0x03, 0x88, 0xd0, 0xf8,
0xa9, 0x7f, 0x20, 0xb4, 0x03, 0xa9, 0x05, 0x8d, 0x24, 0x03, 0xad, 0xcb, 0x03, 0xc9, 0xf6, 0xd0,
0x0a, 0xa9, 0x77, 0x20, 0xb4, 0x03, 0xa9, 0xf9, 0x8d, 0xcb, 0x03, 0xad, 0x01, 0x03, 0xd0, 0x0e,
0xad, 0x00, 0x03, 0xf0, 0x0f, 0xc9, 0xfa, 0xb0, 0x05, 0xe9, 0x00, 0x8d, 0xcb, 0x03, 0x20, 0xb4,
0x03, 0x4c, 0x23, 0x03, 0x85, 0x04, 0xa9, 0x31, 0x4c, 0x65, 0x01, 0x7f, 0x77, 0x7d, 0x75, 0x7b,
0x73, 0x79, 0x71, 0x7e, 0x76, 0x7c, 0x74, 0x7a, 0x72, 0x78, 0x70, 0xf6, 0x00, 0x00, 0x00, 0x00,
0x81, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0xfb, 0x01, 0xfe,
0xa9, 0x6f, 0x8d, 0x0f, 0x04, 0x4c, 0x7b, 0x01, 0xa0, 0x02, 0x8c, 0xd3, 0x01, 0x88, 0xa6, 0x02,
0x20, 0xf7, 0x04, 0x84, 0x56, 0x20, 0x8b, 0x01, 0x8d, 0x00, 0x07, 0x45, 0x56, 0x85, 0x56, 0xce,
0x19, 0x01, 0xd0, 0xf1, 0x20, 0x9f, 0x01, 0x20, 0x4b, 0x03, 0xa0, 0x67, 0x4c, 0xd6, 0x03, 0xd0,
0xf9, 0xad, 0x03, 0x01, 0x20, 0x72, 0x03, 0xc5, 0x21, 0xd0, 0xef, 0xaa, 0xa0, 0x05, 0x94, 0x3e,
0xb8, 0x50, 0xfe, 0xb8, 0x88, 0x10, 0xfa, 0x8c, 0x03, 0x1c, 0xa9, 0xce, 0x8d, 0x0c, 0x1c, 0xa2,
0x06, 0x50, 0xfe, 0xb8, 0x8c, 0x01, 0x1c, 0xca, 0xd0, 0xf7, 0xa0, 0xbb, 0xa2, 0x02, 0x8a, 0x8d,
0x64, 0x01, 0xb9, 0x00, 0x02, 0x50, 0xfe, 0xb8, 0x8d, 0x01, 0x1c, 0xc8, 0xd0, 0xf4, 0xa9, 0x07,
0xca, 0xd0, 0xec, 0x50, 0xfe, 0x20, 0x00, 0xfe, 0x20, 0x4b, 0x03, 0x20, 0x8b, 0x01, 0xa8, 0xd0,
0x87, 0x86, 0x56, 0xa9, 0x18, 0x8d, 0x0f, 0x04, 0x4c, 0xb5, 0x05, 0xa2, 0x94, 0x20, 0x99, 0x01,
0xa9, 0x80, 0xa2, 0x10, 0x20, 0x7a, 0x03, 0xa2, 0x95, 0xec, 0x00, 0x18, 0xd0, 0xfb, 0x60, 0xa9,
0xbb, 0x8d, 0xd2, 0x01, 0xa7, 0x57, 0x20, 0xbc, 0x01, 0xad, 0x00, 0x07, 0x20, 0xbc, 0x01, 0xee,
0xaa, 0x01, 0xd0, 0xf5, 0xa5, 0x56, 0x20, 0xbc, 0x01, 0x20, 0xbc, 0x01, 0x48, 0x4a, 0x4a, 0x4a,
0x4a, 0x20, 0xc7, 0x01, 0x68, 0x29, 0x0f, 0xa8, 0xb9, 0x7f, 0xf7, 0x0a, 0x0a, 0x0a, 0xa0, 0x05,
0x0a, 0x2e, 0xbb, 0x02, 0xca, 0x10, 0x0a, 0xa2, 0x07, 0xee, 0xd2, 0x01, 0xd0, 0x03, 0x8e, 0xd3,
0x01, 0x88, 0xd0, 0xec, 0x60
};