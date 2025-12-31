#include <iostream>
#include "newsaggregator.hpp"

using namespace std;

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
    nag2.AddSubscription(1, 5, 2, vector<string>{"technology"});
    nag2.AddSubscription(2, 5, 2, vector<string>{"technology", "sport"});
    nag2.AddSubscription(3, 1, 100, vector<string>{"sport"});
    nag2.NewsReceived(1, 10.0f, 4, vector<string>{"sport"});
    nag2.NewsReceived(2, 20.0f, 4, vector<string>{"technology"});
    nag2.NewsReceived(3, 30.0f, 5, vector<string>{"sport"});
    nag2.NewsReceived(4, 40.0f, 5, vector<string>{"technology"});
    nag2.NewsReceived(5, 50.0f, 6, vector<string>{"sport", "technology"});
    nag2.NewsReceived(6, 1003.0f, 5, vector<string>{"technology"});
    nag2.NewsReceived(7, 1004.0f, 5, vector<string>{"technology", "sport"});
    map<unsigned int, set<unsigned int>> pubs2 = nag2.Publish(1020.0f, 1000.0f);
    print_publish(pubs2);

    return 0;
}
