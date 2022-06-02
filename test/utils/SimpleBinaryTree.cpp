#include "SimpleBinaryTree.h"
// --------------------------------------------------------------------------
#include <iostream>
#include <new>
// --------------------------------------------------------------------------
namespace utils {
// --------------------------------------------------------------------------
SimpleBinaryTree::SimpleBinaryTree(const std::string& path, std::size_t slowDownUS)
    : slowDownUS(slowDownUS), pageBuffer(path, 1.25) {
    rootID = pageBuffer.createPage();
    auto& rootPage = pageBuffer.pinPage(rootID, true);
    initializeNode(rootPage);
    getNode(rootPage).value = 0;
    pageBuffer.unpinPage(rootID, true);
}
// --------------------------------------------------------------------------
void SimpleBinaryTree::initializeNode(buffer::Page<4096>& page) const {
    new (page.data.data()) Node;
}
// --------------------------------------------------------------------------
SimpleBinaryTree::Node& SimpleBinaryTree::getNode(buffer::Page<4096>& page) const {
    Node* nodePtr = reinterpret_cast<Node*>(page.data.data());
    return *nodePtr;
}
// --------------------------------------------------------------------------
void SimpleBinaryTree::insert(int value) {
    assert(value != 0);
    auto* currentPage = &pageBuffer.pinPage(rootID, true);
    while (true) {
        //std::this_thread::sleep_for(std::chrono::microseconds(slowDownUS));
        if (value <= *getNode(*currentPage).value) {
            if (getNode(*currentPage).leftID) {
                auto* nextPage = &pageBuffer.pinPage(
                        *getNode(*currentPage).leftID, true);
                pageBuffer.unpinPage(currentPage->id, false);
                currentPage = nextPage;
            } else {
                std::uint64_t newPageID = pageBuffer.createPage();
                auto* nextPage = &pageBuffer.pinPage(newPageID, true, true);
                getNode(*currentPage).leftID = newPageID;
                pageBuffer.unpinPage(currentPage->id, true);
                initializeNode(*nextPage);
                getNode(*nextPage).value = value;
                pageBuffer.unpinPage(nextPage->id, true);
                return;
            }
        } else {
            if (getNode(*currentPage).rightID) {
                auto* nextPage = &pageBuffer.pinPage(
                        *getNode(*currentPage).rightID, true);
                pageBuffer.unpinPage(currentPage->id, false);
                currentPage = nextPage;
            } else {
                std::uint64_t newPageID = pageBuffer.createPage();
                auto* nextPage = &pageBuffer.pinPage(newPageID, true, true);
                getNode(*currentPage).rightID = newPageID;
                pageBuffer.unpinPage(currentPage->id, true);
                initializeNode(*nextPage);
                getNode(*nextPage).value = value;
                pageBuffer.unpinPage(nextPage->id, true);
                return;
            }
        }
    }
}
// --------------------------------------------------------------------------
bool SimpleBinaryTree::search(int value) {
    assert(value != 0);
    auto* currentPage = &pageBuffer.pinPage(rootID, true);
    while (true) {
        //std::this_thread::sleep_for(std::chrono::microseconds(slowDownUS));
        if (value <= *getNode(*currentPage).value) {
            if (value == *getNode(*currentPage).value) {
                pageBuffer.unpinPage(currentPage->id, false);
                return true;
            }
            if (getNode(*currentPage).leftID) {
                auto* nextPage = &pageBuffer.pinPage(
                        *getNode(*currentPage).leftID, false);
                pageBuffer.unpinPage(currentPage->id, false);
                currentPage = nextPage;
            } else {
                pageBuffer.unpinPage(currentPage->id, false);
                return false;
            }
        } else {
            if (getNode(*currentPage).rightID) {
                auto* nextPage = &pageBuffer.pinPage(
                        *getNode(*currentPage).rightID, false);
                pageBuffer.unpinPage(currentPage->id, false);
                currentPage = nextPage;
            } else {
                pageBuffer.unpinPage(currentPage->id, false);
                return false;
            }
        }
    }
}
// --------------------------------------------------------------------------
}// namespace utils
// --------------------------------------------------------------------------
