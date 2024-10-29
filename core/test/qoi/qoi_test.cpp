#include "qoi/qoi.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define QOI_TEST_WIDTH 600
#define QOI_TEST_HEIGHT 300
const size_t QOI_TEST_SIZE = QOI_TEST_WIDTH * QOI_TEST_HEIGHT;
const size_t QOI_TEST_BYTE_SIZE = QOI_TEST_SIZE * 4;

bool qoi_test () {
  std::cerr << "Test QOI Codec ";
  void *data = malloc (QOI_TEST_BYTE_SIZE);
  try {
    memset (data, 0x00, QOI_TEST_BYTE_SIZE);
    qoi_desc d{QOI_TEST_WIDTH, QOI_TEST_HEIGHT, 4, 0};
    size_t ch;
    void *en = qoi_encode (data, &d, &ch);
    void *dec = qoi_decode (en, ch, QOI_TEST_BYTE_SIZE, 4);

    if (!memcmp (data, dec, QOI_TEST_BYTE_SIZE)) {
      free (en);
      free (dec);
      throw "failed compression 1";
    }
    free (en);
    free (dec);

    memset (data, 0xff, QOI_TEST_BYTE_SIZE);
    en = qoi_encode (data, &d, &ch);
    dec = qoi_decode (en, ch, QOI_TEST_BYTE_SIZE, 4);

    if (!memcmp (data, dec, QOI_TEST_BYTE_SIZE)) {
      free (en);
      free (dec);
      throw "failed compression 2";
    }
    free (en);
    free (dec);

    memset (data, 0xaa, QOI_TEST_SIZE);
    memset (((unsigned char *)data) + QOI_TEST_SIZE, 0xbb, QOI_TEST_SIZE);
    memset (((unsigned char *)data) + 2 * QOI_TEST_SIZE, 0x44, QOI_TEST_SIZE);
    memset (((unsigned char *)data) + 3 * QOI_TEST_SIZE, 0x88, QOI_TEST_SIZE);
    en = qoi_encode (data, &d, &ch);
    dec = qoi_decode (en, ch, QOI_TEST_BYTE_SIZE, 4);

    if (!memcmp (data, dec, QOI_TEST_BYTE_SIZE)) {
      free (en);
      free (dec);
      throw "failed compression 3";
    }
    free (en);
    free (dec);
  } catch (const char *er) {
    free (data);
    std::cerr << "X :" << er << std::endl;
    return false;
  } catch (...) {
    free (data);
    std::cerr << "X : unchaught error" << std::endl;
    return false;
  }
  free (data);
  std::cerr << "√" << std::endl;
  return true;
}