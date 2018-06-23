/*
 * Copyright 2003-2005 Daniel F. Savarese
 * Copyright 2006-2009 Savarese Software Research Corporation
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

#include <ssrc/spatial/kd_tree.h>

#include <cstdlib>
#include <list>
#include <set>
#include <array>
#include <utility>
#include <ctime>

#define BOOST_TEST_MODULE RangeSearchTreeTest
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

using namespace ssrc::spatial;
using namespace std::rel_ops;

template<typename T>
struct TruePred {
  bool operator()(const T &) { return true; }
};

const unsigned int SquareSize = 16;
// TODO: Move these into TestData and test ranges that span negative values.
const unsigned int MinCoord  = 0;
const unsigned int MaxCoord  = (1 << SquareSize);

template<typename number_type>
number_type random_coord() {
  return (MinCoord + std::rand() % (MaxCoord - MinCoord));
}

template<>
double random_coord<double>() {
  const double scale =
    (static_cast<double>(std::rand()) / (static_cast<double>(RAND_MAX) + 1.0));
  return (static_cast<double>(MinCoord) +
          scale*static_cast<double>(MaxCoord - MinCoord));
}

template<typename V>
struct TestData {
  static const unsigned int NumPoints = 16384;

  typedef V coordinate_type;
  typedef NS_TR1::array<coordinate_type, 2> Point;
  typedef kd_tree<Point, Point*> Tree;
  typedef TruePred<typename Tree::value_type> True;
  typedef std::list<Point *> PointList;
  typedef std::set<Point> PointSet;

  static PointList points;

  TestData() {
    // ensures no duplicate coordinates
    PointSet set;

    std::srand(std::time(0));

    for(unsigned int i = 0; i < NumPoints; ++i) {
      Point point;
      point[0] = random_coord<coordinate_type>();
      point[1] = random_coord<coordinate_type>();
      std::pair<typename PointSet::iterator, bool> result = set.insert(point);

      if(result.second) {
        points.push_back(new Point(point));
      }
    }
  }

  virtual ~TestData() {
    while(!points.empty()) {
      Point *point = points.back();
      points.pop_back();
      delete point;
    }
  }
};

template<class V> typename TestData<V>::PointList TestData<V>::points;

template<typename TD>
struct FillTree {
  typedef typename TD::PointList PointList;
  typedef typename TD::Tree Tree;

  void operator()(Tree & tree, const PointList & points) {
    for(typename PointList::const_iterator p = points.begin();
        p != points.end(); ++p)
      tree[**p] = *p;
  }
};

template<typename TD>
struct FillTreeAndOptimize : public FillTree<TD> {
  typedef FillTree<TD> super;

  void operator()(typename super::Tree & tree,
                  const typename super::PointList & points)
  {
    super::operator()(tree, points);
    tree.optimize();
  }
};

template<typename TD, typename FT>
struct TestScenario {
  typedef TD test_data;
  typedef FT fill_tree;
};

typedef TestData<unsigned int> uint_data;
typedef TestData<int> int_data;
typedef TestData<double> dbl_data;

typedef boost::mpl::list<uint_data, int_data, dbl_data> test_data_types;
typedef
boost::mpl::list<TestScenario<uint_data, FillTree<uint_data> >,
                 TestScenario<uint_data, FillTreeAndOptimize<uint_data> >,
                 TestScenario<int_data, FillTree<int_data> >,
                 TestScenario<int_data, FillTreeAndOptimize<int_data> >,
                 TestScenario<dbl_data, FillTree<dbl_data> >,
                 TestScenario<dbl_data, FillTreeAndOptimize<dbl_data> > >
test_scenario_types;

BOOST_GLOBAL_FIXTURE(uint_data)
BOOST_GLOBAL_FIXTURE(int_data)
BOOST_GLOBAL_FIXTURE(dbl_data)

BOOST_AUTO_TEST_CASE_TEMPLATE(test_begin, test_data, test_data_types) {
  typename test_data::Tree tree;
  typename test_data::Point point{ { 15, 17 } };

  BOOST_REQUIRE(tree.begin() == tree.end());

  tree[point] = &point;

  BOOST_REQUIRE(tree.begin() != tree.end());

  BOOST_REQUIRE(tree.begin()->first == point);
  BOOST_REQUIRE(tree.begin()->second == &point);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_size, test_data, test_data_types) {
  typedef typename test_data::Tree Tree;
  typedef typename test_data::True True;
  typedef typename test_data::PointList PointList;

  const PointList & points = test_data::points;
  Tree tree;
  typename Tree::size_type size = typename Tree::size_type();

  BOOST_REQUIRE(tree.size() == 0);

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    if(!tree.insert(**p, *p))
      ++size;
  }

  BOOST_REQUIRE(tree.size() == size);
  BOOST_REQUIRE(tree.size() <= tree.max_size());
  BOOST_REQUIRE(tree.empty() == (tree.size() == 0));
  BOOST_REQUIRE_EQUAL(tree.size(),
                      static_cast<typename Tree::size_type>(std::count_if(tree.begin(), tree.end(), True())));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_indexed_access, test_data, test_data_types) {
  typedef typename test_data::Tree Tree;
  typedef typename test_data::PointList PointList;

  const PointList & points = test_data::points;
  Tree tree;

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    tree[**p] = *p;
    BOOST_REQUIRE(tree[**p] == *p);
  }
}


BOOST_AUTO_TEST_CASE_TEMPLATE(test_clear, scenario_type, test_scenario_types) {
  typename scenario_type::fill_tree fill_tree;
  typename scenario_type::test_data::Tree tree;

  fill_tree(tree, scenario_type::test_data::points);

  BOOST_REQUIRE(tree.size() > 0);

  tree.clear();

  BOOST_REQUIRE(tree.empty());
  BOOST_REQUIRE(tree.size() == 0);
}

// TODO: Add search of a known subrange
BOOST_AUTO_TEST_CASE_TEMPLATE(test_range_search,
                              scenario_type, test_scenario_types)
{
  typedef typename scenario_type::test_data::Tree Tree;
  typedef typename scenario_type::test_data::True True;
  typedef typename scenario_type::test_data::Point Point;

  typename scenario_type::fill_tree fill_tree;
  Tree tree;

  fill_tree(tree, scenario_type::test_data::points);

  BOOST_REQUIRE(tree.size() ==
                 static_cast<typename Tree::traits::size_type>(
                        std::count_if<typename Tree::iterator, True>(
                        tree.begin(Point{{MinCoord, MinCoord}},
                                   Point{{MaxCoord, MaxCoord}}),
                        tree.end(), True())));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_find, scenario_type, test_scenario_types) {
  typedef typename scenario_type::test_data::Tree Tree;
  typedef typename scenario_type::test_data::PointList PointList;

  const PointList & points = scenario_type::test_data::points;
  typename scenario_type::fill_tree fill_tree;
  Tree tree;
  const Tree & constTree = tree;

  BOOST_REQUIRE(!tree.get(*points.back()));

  BOOST_REQUIRE(tree.find(*points.back()) == tree.end());

  fill_tree(tree, points);

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    typename Tree::mapped_type value;

    BOOST_REQUIRE(tree.get(**p, &value));
    BOOST_REQUIRE(value == *p);

    typename Tree::iterator it = tree.find(**p);

    BOOST_REQUIRE(it != tree.end());
    BOOST_REQUIRE(it->first == **p);
    BOOST_REQUIRE(it->second == *p);

    typename Tree::const_iterator cit = constTree.find(**p);

    BOOST_REQUIRE(cit != constTree.end());
    BOOST_REQUIRE(cit->first == **p);
    BOOST_REQUIRE(cit->second == *p);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_insert, test_data, test_data_types) {
  typedef typename test_data::Tree Tree;
  typedef typename test_data::PointList PointList;

  const PointList & points = test_data::points;
  Tree tree;

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    tree.insert(**p, *p);
    BOOST_REQUIRE(tree.get(**p));
  }

  tree.clear();

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    typename Tree::value_type value(**p, *p);
    std::pair<typename Tree::iterator,bool> pair = tree.insert(value);

    BOOST_REQUIRE(pair.second); 
    BOOST_REQUIRE(tree.get(**p));
    BOOST_REQUIRE(*(pair.first) == value);
  }

  typename Tree::mapped_type original, replaced;

  BOOST_REQUIRE(tree.get(*points.back(), &original));
  BOOST_REQUIRE(tree.insert(*points.back(), original, &replaced));
  BOOST_REQUIRE(original == replaced);

  std::pair<typename Tree::iterator,bool> result;

  result = tree.insert(typename Tree::value_type(*points.back(), replaced));
  BOOST_REQUIRE(!result.second);
  BOOST_REQUIRE(result.first->second == replaced);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_assignment, test_data, test_data_types) {
  typedef typename test_data::Tree Tree;
  typedef typename test_data::Point Point;
  typedef typename test_data::PointList PointList;

  const PointList & points = test_data::points;
  Tree tree1;

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    tree1.insert(**p, *p);
  }

  BOOST_REQUIRE(tree1 == tree1);

  // Test copy constructor.
  Tree tree2(tree1);

  BOOST_REQUIRE(tree2 == tree1);

  // Test assignment operator.

  Tree tree3;

  tree3.insert(**points.begin(), *points.begin());

  BOOST_REQUIRE(tree3 != tree1);

  tree3 = tree1;

  BOOST_REQUIRE(tree3 == tree1);

  tree3.erase(**points.rbegin());

  BOOST_REQUIRE(tree3 != tree1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_erase, scenario_type, test_scenario_types) {
  typedef typename scenario_type::test_data::Tree Tree;
  typedef typename scenario_type::test_data::PointList PointList;

  const PointList & points = scenario_type::test_data::points;
  typename scenario_type::fill_tree fill_tree;
  Tree tree;

  fill_tree(tree, points);

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    typename Tree::mapped_type erased;
    BOOST_REQUIRE(tree.remove(**p, &erased));
    BOOST_REQUIRE(erased == *p);
  }

  BOOST_REQUIRE(tree.empty());
  BOOST_REQUIRE(tree.size() == 0);

  fill_tree(tree, points);

  for(typename PointList::const_iterator p = points.begin();
      p != points.end(); ++p)
  {
    tree.erase(tree.find(**p));
  }

  BOOST_REQUIRE(tree.empty());
  BOOST_REQUIRE(tree.size() == 0);

  BOOST_REQUIRE(tree.erase(*points.back()) == 0);

  fill_tree(tree, points);

  typename Tree::size_type size = tree.size();
  typename Tree::size_type erased = 0;
  typename Tree::size_type count = 0;
  for(typename Tree::iterator p = tree.begin(); p != tree.end(); ++count) {
    if(std::rand() % 2) {
      p = tree.erase(p);
      ++erased;
    } else {
      ++p;
    }
  }

  BOOST_REQUIRE(tree.size() == size - erased);
  BOOST_REQUIRE_EQUAL(size, count);

  tree.clear();
  fill_tree(tree, points);

  size = tree.size();
  count =  0;

  for(typename Tree::iterator p = tree.begin(); p != tree.end(); ++count) {
    p = tree.erase(p);
  }

  BOOST_REQUIRE(tree.empty());
  BOOST_REQUIRE(size == count);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_find_nearest_neighbor, test_data,
                              test_data_types)
{
  typedef typename test_data::Tree Tree;
  typedef typename test_data::Point Point;
  typedef typename test_data::PointList PointList;
  typedef typename Tree::knn_iterator knn_iterator;
  typedef typename test_data::coordinate_type coordinate_type;

  Tree tree;
  Point query{{1,1}};

  BOOST_REQUIRE(tree.find_nearest_neighbor(query) == tree.end());

  tree.insert(Point{{1,1}}, 0);

  BOOST_REQUIRE(tree.find_nearest_neighbor(query) == tree.end());

  tree.insert(Point{{100,100}}, 0);

  typename Tree::iterator it = tree.find_nearest_neighbor(query);

  BOOST_REQUIRE(it != tree.end());
  BOOST_REQUIRE((it->first == Point{{100,100}}));

  tree.insert(Point{{3,3}}, 0);

  it = tree.find_nearest_neighbor(query);

  BOOST_REQUIRE(it != tree.end());
  BOOST_REQUIRE((it->first == Point{{3,3}}));

  std::pair<knn_iterator,knn_iterator> range =
    tree.find_nearest_neighbors(query, 1);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == it->first));

  FillTree<test_data> fill_tree;

  tree.clear();

  fill_tree(tree, test_data::points);
  tree.insert(Point{{2,2}}, 0);

  it = tree.find_nearest_neighbor(query);

  BOOST_REQUIRE(it != tree.end());
  BOOST_REQUIRE((it->first == Point{{2,2}} ||
                 euclidean_distance<Point>::d2(query, it->first) < 2));

  range = tree.find_nearest_neighbors(query, 1);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == it->first));

  it = tree.find_nearest_neighbor(Point{{2,2}}, false);

  BOOST_REQUIRE(it != tree.end());
  BOOST_REQUIRE((it->first == Point{{2,2}}));

  range = tree.find_nearest_neighbors(query, 1);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == it->first));

  // Now a brute force check.  Note, we don't account for the possiblity
  // of ties, which may cause a false failure on rare occasion.
  tree.clear();
  fill_tree(tree, test_data::points);
  Point *nearest = 0;
  double min = detail::coordinate_limits<double>::highest();

  query[0] = query[1] = MaxCoord / 2;

  for(typename PointList::const_iterator p = test_data::points.begin();
      p != test_data::points.end(); ++p)
  {
    const double ed = euclidean_distance<Point>::d2(query, **p);
    if(ed < min) {
      min = ed;
      nearest = *p;
    }
  }

  BOOST_REQUIRE(nearest != 0);

  it = tree.find_nearest_neighbor(query);

  BOOST_REQUIRE(it != tree.end());
  BOOST_REQUIRE(it->first == *nearest);

  range = tree.find_nearest_neighbors(query, 1);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == it->first));

  query[0] = query[1] = -1;

  // This should be a separate regression test.  It catches the bug
  // whereby nearest neighbors tests for k = 1 would produce an
  // incorrect result when k > 2 would be correct.
  // Skip for unsigned tests.
  if(query[0] < 0) {
    Point data[6] = { {{0, 0}}, {{ 3, 1}}, {{4, 2}}, {{1, 1}},
                      {{static_cast<coordinate_type>(0.5),
                        static_cast<coordinate_type>(1.5)}},
                      {{static_cast<coordinate_type>(4.8),
                        static_cast<coordinate_type>(4.9)}} };
    tree.clear();

    for(unsigned int i = 0; i < 6; ++i) {
      tree.insert(data[i], &data[i]);
    }

    it = tree.find_nearest_neighbor(query, 1);

    BOOST_REQUIRE(it != tree.end());
    BOOST_REQUIRE((it->first == Point{{0,0}}));

    range = tree.find_nearest_neighbors(query, 2);

    BOOST_REQUIRE(range.first != range.second);

    range = tree.find_nearest_neighbors(query, 1);

    BOOST_REQUIRE(range.first != range.second);
    BOOST_REQUIRE(range.first->first == it->first);
  }
}

template<typename Point>
struct knn_comp {
  const Point & query;

  knn_comp(const Point & query) : query(query) { }

  bool operator()(const Point * const p1, const Point * const p2) {
    return (euclidean_distance<Point>::d2(query, *p1) <
            euclidean_distance<Point>::d2(query, *p2));
  }
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_find_nearest_neighbors, test_data,
                              test_data_types)
{
  typedef typename test_data::Tree Tree;
  typedef typename Tree::knn_iterator knn_iterator;
  typedef typename test_data::Point Point;
  typedef typename test_data::PointList PointList;

  Point query{{MaxCoord / 2, MaxCoord /2}};
  Tree tree;
  FillTree<test_data> fill_tree;

  fill_tree(tree, test_data::points);

  // Brute force sorting of points by distance for use as expected values.
  // Note, we may get occasional false failures in the presence of ties
  // because we use an unstable sort.
  std::vector<Point *> sorted_points(test_data::points.begin(),
                                     test_data::points.end());
  std::sort(sorted_points.begin(), sorted_points.end(), knn_comp<Point>(query));

  for(unsigned int i = 1; i < 11; ++i) {
    std::pair<knn_iterator,knn_iterator> range =
      tree.find_nearest_neighbors(query, i, false);
    unsigned int j = 0;
    BOOST_REQUIRE(range.first != range.second);
    for(knn_iterator & it = range.first, & end = range.second; it != end; ++it)
    {
      BOOST_REQUIRE(it->first == *sorted_points[j++]);
    }
  }

  for(unsigned int i = 1; i < 11; ++i) {
    std::pair<knn_iterator,knn_iterator> range =
      tree.find_nearest_neighbors(query, i, true);
    unsigned int j = 0;
    if(*sorted_points[0] == query) {
      ++j;
    }
    BOOST_REQUIRE(range.first != range.second);
    for(knn_iterator & it = range.first, & end = range.second; it != end; ++it)
    {
      BOOST_REQUIRE(it->first == *sorted_points[j++]);
    }
  }

  // Verify degenerate case.

  query[0] = query[1] = 1; 
  tree.insert(Point{{2,2}}, 0);

  std::pair<knn_iterator,knn_iterator> range =
    tree.find_nearest_neighbors(query, 1);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == Point{{2,2}} ||
                 euclidean_distance<Point>::d2(query, range.first->first) < 2));

  query[0] = query[1] = 2; 
  range = tree.find_nearest_neighbors(query, 1, false);

  BOOST_REQUIRE(range.first != range.second);
  BOOST_REQUIRE((range.first->first == Point{{2,2}}));
}
