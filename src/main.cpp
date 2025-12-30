#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>

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



class NewsAggregator {
public:
    NewsAggregator() = default;

    bool AddSubscription(unsigned int id, unsigned int minInterest, unsigned int maxNewsPerSecond, vector<string> && topics) {
        Subscriber sub {minInterest, maxNewsPerSecond, move(topics)};
        // create a map of news assigned to this subsciber - sorted by timestamp
        subnews_.insert_or_assign(id, map<float, unsigned int>());
        for(const auto & t : sub.topics) {
            topics_subs_[t].push_back(id); // bookeeping this user belongs to this topic
            // check if we have existing news in this topic, if yes, assign them to this user if interested
            if (topics_news_.contains(t)) {
                for (auto news_id : topics_news_.at(t)) {
                    if (news_.at(news_id).interest >= sub.minInterest) {
                        subnews_.at(id).insert({news_.at(news_id).timestamp, news_id});
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
                if (subs_[sub_id].minInterest >= news.interest) {
                    subnews_[sub_id].insert({news.timestamp, id});
                }
            }
        }

        news_.insert({id, move(news)});
        return true;
    }

    void Publish(float timestamp, float maxAge) {
        float start_t = timestamp - maxAge;
        float end_t = timestamp;

        vector<pair<unsigned int, vector<unsigned>>> sub_news_filtered;
        for (const auto & subnews : subnews_) {
            vector<unsigned int> news_in_range;
            auto itStart = subnews.second.lower_bound(start_t);
            auto itEnd = subnews.second.lower_bound(end_t);
            for (auto it = itStart; it != itEnd; ++it) {
                news_in_range.push_back(it->second);
            }
            if (news_in_range.size() <= subs_[subnews.first].maxNewsPerSecond) {
                sub_news_filtered.push_back({subnews.first, news_in_range});
            }
        }
    }

private:
    unordered_map<unsigned int, Subscriber> subs_;
    unordered_map<unsigned int, News> news_;
    unordered_map<string, vector<unsigned int>> topics_news_; // bookeeping for news assigned to topics
    unordered_map<string, vector<unsigned int>> topics_subs_; // bookeeping for subs assigned to topics
    unordered_map<unsigned int, map<float, unsigned int>> subnews_;
};


int main() {
    cout << "hellow news" << endl;

    return 0;
}
