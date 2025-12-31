#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>

using namespace std;

struct Subscriber {
    unsigned int minInterest;
    unsigned int maxNewsPerSecond;
    vector<string> topics;
};

struct News {
    float timestamp;
    unsigned int interest;
    vector<string> topics;
};

struct News_metadata {
    float timestamp;
    unsigned int interest;
    unsigned int id;
};

class NewsAggregator {
public:
    NewsAggregator() = default;

    bool AddSubscription(unsigned int id, unsigned int minInterest, unsigned int maxNewsPerSecond, vector<string> && topics) {
        Subscriber sub {minInterest, maxNewsPerSecond, move(topics)};
        // create a map of news assigned to this subsciber - sorted by timestamp
        subnews_.insert_or_assign(id, multimap<float, News_metadata>());
        for(const auto & t : sub.topics) {
            topics_subs_[t].push_back(id); // bookeeping this user belongs to this topic
            // check if we have existing news in this topic, if yes, assign them to this user if interested
            if (topics_news_.contains(t)) {
                for (auto news_id : topics_news_.at(t)) {
                    const News & n = news_.at(news_id);
                    if (n.interest >= sub.minInterest) {
                        News_metadata md{n.timestamp, n.interest, news_id};
                        subnews_.at(id).insert({news_.at(news_id).timestamp, move(md)});
                    }
                }
            }
        }

        subs_.insert_or_assign(id, move(sub));

        return true; // todo: impelement proper error handling, maybe try catch for the maps manipuations
    }

    bool NewsReceived(unsigned int id, float timestamp, unsigned int interest, vector<string> && topics) {
        if (news_.contains(id)) {
            return false;
        }
        
        News news {timestamp, interest, move(topics)};

        for (const auto & t : news.topics) {
            topics_news_[t].push_back(id); // bookeeping this news belongs to these topics
            // assings this new to the users who are interested in it
            for (const auto & sub_id : topics_subs_[t]) {
                if (subs_[sub_id].minInterest <= news.interest) {
                    News_metadata md{news.timestamp, news.interest, id};
                    subnews_[sub_id].insert({news.timestamp, move(md)});
                }
            }
        }

        news_.insert({id, move(news)});
        return true;
    }

    map<unsigned int, set<unsigned int>> Publish(float timestamp, float maxAge) {
        float start_t = timestamp - maxAge;
        float end_t = timestamp;

        map<unsigned int, set<unsigned int>> news_to_users; 

        // generate the news for every user
        vector<pair<unsigned int, vector<unsigned>>> sub_news_filtered; // vec<(sub_id, vec<news_id>)>, todo: storing this is unnecessary
        for (const auto & subnews : subnews_) {
            // get the metadatas of the news in the given time range
            vector<News_metadata> news_in_range;
            auto itStart = subnews.second.lower_bound(start_t);
            auto itEnd = subnews.second.lower_bound(end_t);
            for (auto it = itStart; it != itEnd; ++it) {
                news_in_range.push_back(it->second);
            }

            const Subscriber & sub = subs_[subnews.first];
            vector<unsigned> filtered_news_ids(sub.maxNewsPerSecond);
            // all the news in the time range can be displayed
            if (news_in_range.size() <= sub.maxNewsPerSecond) {
                for (const auto & md : news_in_range) {
                    filtered_news_ids.push_back(md.id);
                    news_to_users[md.id].insert(subnews.first); // converting: user gets new -> new gets this user
                }
            // news need to be sorted
            } else {
                vector<unsigned> filtered_idxs_desc(news_in_range.size());
                iota(filtered_idxs_desc.begin(), filtered_idxs_desc.end(), 0); // indexes
                sort(filtered_idxs_desc.begin(), filtered_idxs_desc.end(), [&](int a, int b){
                    return tie(news_in_range[a].interest, news_in_range[a].timestamp, news_in_range[a].timestamp) > 
                           tie(news_in_range[b].interest, news_in_range[b].timestamp, news_in_range[b].timestamp);
                });
                
                for (int i = 0; i < filtered_news_ids.size(); ++i) {
                    filtered_news_ids[i] = news_in_range[filtered_idxs_desc[i]].id;
                    news_to_users[news_in_range[filtered_idxs_desc[i]].id].insert(subnews.first); // converting: user gets new -> new gets this user
                }
            }
            sub_news_filtered.push_back({subnews.first, move(filtered_news_ids)});
        }

        // convert the user:[news] to news:[user]
        return news_to_users;
    }

private:
    unordered_map<unsigned int, Subscriber> subs_;
    unordered_map<unsigned int, News> news_;
    unordered_map<string, vector<unsigned int>> topics_news_; // bookeeping for news assigned to topics
    unordered_map<string, vector<unsigned int>> topics_subs_; // bookeeping for subs assigned to topics
    unordered_map<unsigned int, multimap<float, News_metadata>> subnews_;
};

void print(const std::map<unsigned int, std::set<unsigned int>>& m) {
    for (const auto& pair : m) {
        std::cout << pair.first << " : { ";
        for (auto val : pair.second) {
            std::cout << val << " ";
        }
        std::cout << "}\n";
    }
}

int main() {
    cout << "hellow news" << endl;

    NewsAggregator nag;
    nag.AddSubscription(1, 5, 2, vector<string>{"radio", "television"});
    nag.AddSubscription(2, 7, 3, vector<string>{"cable"});
    nag.NewsReceived(10, 100.0f, 4, vector<string>{"television"});
    nag.NewsReceived(11, 100.0f, 5, vector<string>{"television", "cable"});
    nag.NewsReceived(12, 150.0f, 7, vector<string>{"television", "streaming"});
    map<unsigned int, set<unsigned int>> pubs = nag.Publish(200.0f, 100.0f);
    print(pubs);

    cout << endl;

    NewsAggregator nag2;
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
    map<unsigned int, set<unsigned int>> pubs2 = nag.Publish(1020.0f, 1000.0f);
    print(pubs2);

    return 0;
}
