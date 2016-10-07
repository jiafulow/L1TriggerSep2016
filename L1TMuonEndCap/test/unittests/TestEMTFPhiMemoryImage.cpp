#include "Utilities/Testing/interface/CppUnit_testdriver.icpp"
#include "cppunit/extensions/HelperMacros.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPhiMemoryImage.hh"


class TestEMTFPhiMemoryImage: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestEMTFPhiMemoryImage);
  CPPUNIT_TEST(test_bitset);
  CPPUNIT_TEST(test_rotation);
  CPPUNIT_TEST(test_out_of_range);
  CPPUNIT_TEST_SUITE_END();

public:
  TestEMTFPhiMemoryImage() {}
  ~TestEMTFPhiMemoryImage() {}
  void setUp() {}
  void tearDown() {}

  void test_bitset();
  void test_rotation();
  void test_out_of_range();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(TestEMTFPhiMemoryImage);


void TestEMTFPhiMemoryImage::test_bitset()
{
  EMTFPhiMemoryImage image;

  image.set_bit(1, 31);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31), true);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 0), 1ul<<31);

  image.clear_bit(1, 31);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31), false);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 0), 0ul);

  image.set_bit(1, 31+64);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31+64), true);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 1), 1ul<<31);

  image.clear_bit(1, 31+64);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31+64), false);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 1), 0ul);

  image.set_bit(1, 31+128);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31+128), true);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 2), 1ul<<31);

  image.clear_bit(1, 31+128);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 31+128), false);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 2), 0ul);

  image.set_bit(1, 57+64);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 57+64), true);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 1), 1ul<<57);

  image.clear_bit(1, 57+64);
  CPPUNIT_ASSERT_EQUAL(image.test_bit(1, 57+64), false);
  CPPUNIT_ASSERT_EQUAL(image.get_word(1, 1), 0ul);

  image.set_bit(3, 99);
  image.reset();
  CPPUNIT_ASSERT_EQUAL(image.test_bit(3, 99), false);
}

void TestEMTFPhiMemoryImage::test_rotation()
{
  EMTFPhiMemoryImage image;

  uint64_t word0 = 0x0000000011111111;
  uint64_t word1 = 0x2222222233333333;
  uint64_t word2 = 0x4444444455555555;
  uint64_t word3 = 0x6666666677777777;
  uint64_t word4 = 0x8888888899999999;
  uint64_t word5 = 0xAAAAAAAABBBBBBBB;
  uint64_t word6 = 0xCCCCCCCCDDDDDDDD;
  uint64_t word7 = 0xEEEEEEEEFFFFFFFF;
  uint64_t word8 = 0x0;
  image.set_word(0, 0, word0);
  image.set_word(0, 1, word1);
  image.set_word(0, 2, word2);
  image.set_word(1, 0, word3);
  image.set_word(1, 1, word4);
  image.set_word(1, 2, word5);
  image.set_word(2, 0, word6);
  image.set_word(2, 1, word7);
  image.set_word(2, 2, word8);

  for (int i = 0; i < 200; ++i) {
    image.rotl(i);

    if (i == 0)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word0 << i) | (word0 >> (64-i)));
    else if (i < 64)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word0 << i) | (word2 >> (64-i)));
    else if (i == 64)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word2 << (i-64)) | (word2 >> (128-i)));
    else if (i < 128)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word2 << (i-64)) | (word1 >> (128-i)));
    else if (i == 128)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word1 << (i-128)) | (word1 >> (192-i)));
    else if (i < 192)
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word1 << (i-128)) | (word0 >> (192-i)));
    else
      CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), (word0));
    
    image.rotr(i);

    CPPUNIT_ASSERT_EQUAL(image.get_word(0, 0), word0);
    CPPUNIT_ASSERT_EQUAL(image.get_word(0, 1), word1);
    CPPUNIT_ASSERT_EQUAL(image.get_word(0, 2), word2);
    CPPUNIT_ASSERT_EQUAL(image.get_word(1, 0), word3);
    CPPUNIT_ASSERT_EQUAL(image.get_word(1, 1), word4);
    CPPUNIT_ASSERT_EQUAL(image.get_word(1, 2), word5);
    CPPUNIT_ASSERT_EQUAL(image.get_word(2, 0), word6);
    CPPUNIT_ASSERT_EQUAL(image.get_word(2, 1), word7);
    CPPUNIT_ASSERT_EQUAL(image.get_word(2, 2), word8);
    CPPUNIT_ASSERT_EQUAL(image.get_word(3, 0), word8);
    CPPUNIT_ASSERT_EQUAL(image.get_word(3, 1), word8);
    CPPUNIT_ASSERT_EQUAL(image.get_word(3, 2), word8);
  }

}

void TestEMTFPhiMemoryImage::test_out_of_range()
{
  EMTFPhiMemoryImage image;

  CPPUNIT_ASSERT_THROW(image.set_word(0, 3, 0x0), std::out_of_range);
  CPPUNIT_ASSERT_THROW(image.get_word(1, 4), std::out_of_range);
  CPPUNIT_ASSERT_THROW(image.set_bit(5, 0), std::out_of_range);
  CPPUNIT_ASSERT_THROW(image.test_bit(5, 4), std::out_of_range);
}
