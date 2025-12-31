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
        sub_news_.insert_or_assign(id, multimap<float, News_metadata>());
        for(const auto & t : sub.topics) {
            topic_subs_[t].push_back(id); // bookeeping this user belongs to this topic
            // check if we have existing news in this topic, if yes, assign them to this user if interested
            if (topic_news_.contains(t)) {
                for (auto news_id : topic_news_.at(t)) {
                    const News & n = news_.at(news_id);
                    if (n.interest >= sub.minInterest) {
                        News_metadata md{n.timestamp, n.interest, news_id};
                        sub_news_.at(id).insert({news_.at(news_id).timestamp, move(md)});
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
            topic_news_[t].push_back(id); // bookeeping this news belongs to these topics
            // assings this new to the users who are interested in it
            for (const auto & sub_id : topic_subs_[t]) {
                if (subs_[sub_id].minInterest <= news.interest) {
                    News_metadata md{news.timestamp, news.interest, id};
                    // check if this key-val already exists (this new was alreaddy added by another topic)
                    auto range = sub_news_[sub_id].equal_range(news.timestamp);
                    bool exists = std::any_of(range.first, range.second,
                        [&](auto const& elem) {
                            return (elem.second.id == md.id);
                        });

                    if (!exists) {
                        sub_news_[sub_id].insert({news.timestamp ,move(md)});
                    }
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

        for (const auto & subnews : sub_news_) {
            // get the metadatas of the news in the given time range
            vector<const News_metadata *> news_in_range;
            auto itStart = subnews.second.lower_bound(start_t);
            auto itEnd = subnews.second.lower_bound(end_t);
            for (auto it = itStart; it != itEnd; ++it) {
                news_in_range.push_back(&it->second);
            }

            const Subscriber & sub = subs_[subnews.first];
            // all the news in the time range can be displayed
            if (news_in_range.size() <= sub.maxNewsPerSecond) {
                for (const auto md : news_in_range) {
                    news_to_users[md->id].insert(subnews.first); // converting: user gets new -> new gets this user
                }
            // news need to be sorted
            } else {
                sort(news_in_range.begin(), news_in_range.end(), [](const News_metadata * a, const News_metadata * b){
                    if (a->interest != b->interest) {
                        return a->interest > b->interest;
                    } else if (a->timestamp != b->timestamp) {
                        return a->timestamp < b->timestamp;
                    } else {
                        return a->id > b->id;
                    }
                });
                
                for (int i = 0; i < sub.maxNewsPerSecond; ++i) {
                    news_to_users[news_in_range[i]->id].insert(subnews.first); // converting: user gets new -> new gets this user
                }
            }
        }

        return news_to_users;
    }

private:
    unordered_map<unsigned int, Subscriber> subs_;
    unordered_map<unsigned int, News> news_;
    unordered_map<string, vector<unsigned int>> topic_news_; // bookeeping for news assigned to topics
    unordered_map<string, vector<unsigned int>> topic_subs_; // bookeeping for subs assigned to topics
    unordered_map<unsigned int, multimap<float, News_metadata>> sub_news_; // todo: maybe just use multimat<ts, newsid> and deprecate news_metadata
};

void print_publish(const std::map<unsigned int, std::set<unsigned int>>& m) {
    std::cout << "publish: " << std::endl;
    for (const auto& pair : m) {
        std::cout << "- news=" << pair.first << " to [";
        for (auto val : pair.second) {
            std::cout << val << " ";
        }
        std::cout << "]\n";
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
    print_publish(pubs);

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
    print_publish(pubs2);

    return 0;
}
