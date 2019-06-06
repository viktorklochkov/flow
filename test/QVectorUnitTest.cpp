
#include "gtest/gtest.h"
#include <array>
TEST(QVectorUnitTest, test) {
  static constexpr std::array<unsigned char, 8> kharmonicmask = {0x01, // 0000 0001
                                                                0x02, // 0000 0010
                                                                0x04, // 0000 0100
                                                                0x08, // 0000 1000
                                                                0x10, // 0001 0000
                                                                0x20, // 0010 0000
                                                                0x40, // 0100 0000
                                                                0x80  // 1000 0000
  };
  unsigned char bits = 0x00;

  bits |= kharmonicmask[0];
  bits |= kharmonicmask[2];
  unsigned int nharmonics = 2;
  unsigned int position = 0;
  for (unsigned int h = 0; h < nharmonics+1; ++h) {
    if ((bits & kharmonicmask[h])) {
      std::cout << h << position << std::endl;
      ++position;
    }
  }

  std::vector<int> q{1,2,3};
  std::bitset<3> bit;
  bit.set(1);
  bit.set(2);
  EXPECT_EQ(1,std::bitset<3>(bit & std::bitset<3>((1UL << (2 + 1)) - 1)).count()-1);


  EXPECT_EQ(true, bits & kharmonicmask[0]);
}