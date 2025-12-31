#include <gtest/gtest.h>
#include "newsaggregator.hpp"

using namespace std;

TEST(NEwsAggregator, Simple) {
    NewsAggregator nag;
    nag.AddSubscription(1, 5, 2, vector<string>{"radio", "television"});
    nag.AddSubscription(2, 7, 3, vector<string>{"cable"});
    nag.NewsReceived(10, 100.0f, 4, vector<string>{"television"});
    nag.NewsReceived(11, 100.0f, 5, vector<string>{"television", "cable"});
    nag.NewsReceived(12, 150.0f, 7, vector<string>{"television", "streaming"});
    map<unsigned int, set<unsigned int>> pubs = nag.Publish(200.0f, 100.0f);

    map<unsigned int, set<unsigned int>> pubs_res{{11, {1}}, {12, {1}}};
    EXPECT_EQ(pubs, pubs_res);
}

TEST(NEwsAggregator, Sort) {
    NewsAggregator nag;
    nag.AddSubscription(1, 5, 2, vector<string>{"technology"});
    nag.AddSubscription(2, 5, 2, vector<string>{"technology", "sport"});
    nag.AddSubscription(3, 1, 100, vector<string>{"sport"});
    nag.NewsReceived(1, 10.0f, 4, vector<string>{"sport"});
    nag.NewsReceived(2, 20.0f, 4, vector<string>{"technology"});
    nag.NewsReceived(3, 30.0f, 5, vector<string>{"sport"});
    nag.NewsReceived(4, 40.0f, 5, vector<string>{"technology"});
    nag.NewsReceived(5, 50.0f, 6, vector<string>{"sport", "technology"});
    nag.NewsReceived(6, 1003.0f, 5, vector<string>{"technology"});
    nag.NewsReceived(7, 1004.0f, 5, vector<string>{"technology", "sport"});
    map<unsigned int, set<unsigned int>> pubs = nag.Publish(1020.0f, 1000.0f);
    
    map<unsigned int, set<unsigned int>> pubs_res{{3, {2, 3}},
                                                  {4, {1}},
                                                  {5, {1, 2, 3}},
                                                  {7, {3}}};
    EXPECT_EQ(pubs, pubs_res);
}

TEST(NEwsAggregator, RandomAddition) {
    NewsAggregator nag;
    nag.AddSubscription(3, 1, 100, vector<string>{"sport"});
    nag.NewsReceived(1, 10.0f, 4, vector<string>{"sport"});
    nag.NewsReceived(2, 20.0f, 4, vector<string>{"technology"});
    nag.NewsReceived(3, 30.0f, 5, vector<string>{"sport"});
    nag.AddSubscription(2, 5, 2, vector<string>{"technology", "sport"});
    nag.NewsReceived(4, 40.0f, 5, vector<string>{"technology"});
    nag.NewsReceived(5, 50.0f, 6, vector<string>{"sport", "technology"});
    nag.NewsReceived(6, 1003.0f, 5, vector<string>{"technology"});
    nag.AddSubscription(1, 5, 2, vector<string>{"technology"});
    nag.NewsReceived(7, 1004.0f, 5, vector<string>{"technology", "sport"});
    map<unsigned int, set<unsigned int>> pubs = nag.Publish(1020.0f, 1000.0f);
    
    map<unsigned int, set<unsigned int>> pubs_res{{3, {2, 3}},
                                                  {4, {1}},
                                                  {5, {1, 2, 3}},
                                                  {7, {3}}};
    EXPECT_EQ(pubs, pubs_res);
}