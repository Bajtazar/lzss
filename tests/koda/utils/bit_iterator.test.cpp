#include <gtest/gtest.h>

#include <koda/utils/bit_iterator.hpp>

#include <sstream>
#include <string>

// TEST(RegionCoordsTest, CoordsToRegionCoords) {
//   auto null_result = anvil::CoordsToRegionCoords({});
//   const anvil::RegionCoords null_coords{};
//   ASSERT_EQ(null_result, null_coords);

//   auto positive_result = anvil::CoordsToRegionCoords({16, 0, 32});
//   const anvil::RegionCoords positive_coords{0, 1};
//   ASSERT_EQ(positive_result, positive_coords);

//   auto positive_result_v2 = anvil::CoordsToRegionCoords({32, 0, 35});
//   const anvil::RegionCoords positive_coords_v2{1, 1};
//   ASSERT_EQ(positive_result_v2, positive_coords_v2);

//   auto negative_result = anvil::CoordsToRegionCoords({-16, 0, 32});
//   const anvil::RegionCoords negative_coords{-1, 1};
//   ASSERT_EQ(negative_result, negative_coords);

//   auto negative_result_v2 = anvil::CoordsToRegionCoords({-32, 0, 35});
//   const anvil::RegionCoords negative_coords_v2{-2, 1};
//   ASSERT_EQ(negative_result_v2, negative_coords_v2);

//   auto negative_result_v3 = anvil::CoordsToRegionCoords({-35, 0, 35});
//   ASSERT_EQ(negative_result_v3, negative_coords_v2);
// }

// static std::string ToString(const anvil::RegionCoords& coords) {
//   std::ostringstream oss{};
//   oss << coords;
//   return oss.str();
// }

// TEST(RegionCoordsTest, OstreamCoords) {
//   const anvil::RegionCoords null_coords{};
//   ASSERT_EQ(ToString(null_coords), "Region Coords(x=0, z=0)");

//   const anvil::RegionCoords positive_coords{0, 1};
//   ASSERT_EQ(ToString(positive_coords), "Region Coords(x=0, z=1)");

//   const anvil::RegionCoords negative_coords{-1, 1};
//   ASSERT_EQ(ToString(negative_coords), "Region Coords(x=-1, z=1)");
// }
