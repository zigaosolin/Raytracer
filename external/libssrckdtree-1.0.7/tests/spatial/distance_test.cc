/*
 * Copyright 2010 Savarese Software Research Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.savarese.com/software/ApacheLicense-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <ssrc/spatial/distance.h>

#include <array>

#define BOOST_TEST_MODULE DistanceTest
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

using namespace ssrc::spatial;

typedef boost::mpl::list<unsigned int, int, double> coordinate_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_d2_0, coordinate_type, coordinate_types) {
  typedef NS_TR1::array<coordinate_type, 1> Point;

  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1}}, Point{{1}}), 0);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1}}, Point{{2}}), 1);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{48}},
                                                    Point{{52}}), 16);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{4}},
                                                    Point{{1}}), 9);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_d2_1, coordinate_type, coordinate_types) {
  typedef NS_TR1::array<coordinate_type, 2> Point;

  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1}},
                                                    Point{{1,1}}), 0);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1}},
                                                    Point{{2,2}}), 2);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{83,9451}},
                                                    Point{{4382,2383}}),
                      68438025);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_d2_2, coordinate_type, coordinate_types) {
  typedef NS_TR1::array<coordinate_type, 3> Point;

  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1,1}},
                                                    Point{{1,1,1}}), 0);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1,1}},
                                                    Point{{2,2,2}}), 3);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{9,0,4}},
                                                    Point{{100,32,0}}), 9321);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_d2_4, coordinate_type, coordinate_types) {
  typedef NS_TR1::array<coordinate_type, 4> Point;

  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1,1,1}},
                                                    Point{{1,1,1,1}}), 0);
  BOOST_REQUIRE_EQUAL(euclidean_distance<Point>::d2(Point{{1,1,1,1}},
                                                    Point{{2,2,2,2}}), 4);
}
