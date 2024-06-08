#include "media.hpp"
#include <QDebug>
#include <qnamespace.h>
#include <qsize.h>

using namespace kedia;
using namespace std;

// Helper function that turns a given media type into a string.
QString kedia::MediaTypeToString(MediaType t) {
    switch (t) {
    case SERIES:
        return "Series";
    case SEASON:
        return "Season";
    case EPISODE:
        return "Episode";
    case NONE:
        return "None";
    case ANY:
        return "Any";
    default:
        return "ERROR";
    }
};

bool Media::hasChild(std::weak_ptr<Media> child_to_check) {
    for (auto& child : getChildren()) {
        if (child_to_check.lock()->id == child.lock()->id) {
            return true;
        }
    }
    return false;
}

std::vector<std::weak_ptr<Media>> Media::getChildren() {
    return sharedToWeak<Media>(children);
}

std::vector<std::weak_ptr<Media>> Media::getParents() {
    return sharedToWeak<Media>(parents);
}

bool Media::hasChildren() { return (countChildren() > 1); }

std::weak_ptr<Media> Media::getChild(int index) {
    if (index < children.size()) {
        return children[index];
    }
    return std::weak_ptr<Media>();
}

std::weak_ptr<Media> Media::getParent(int index) {
    if (index < parents.size()) {
        return parents[index];
    }
    return std::weak_ptr<Media>();
}

int Media::childIndex(std::weak_ptr<Media> child) {

    shared_ptr<Media> target_child = child.lock();
    for (int i = 0; i < children.size(); i++) {
        auto found_child = children[i];
        if (found_child == target_child) {
            return i;
        }
    }
    return -1;
}

int Media::parentIndex(std::weak_ptr<Media> parent) {

    if (parent.expired()) {
        return -1;
    }
    shared_ptr<Media> target_parent = parent.lock();
    for (int i = 0; i < parents.size(); i++) {
        auto found_parent = parents[i];
        if (found_parent->getId() == target_parent->getId()) {
            return i;
        }
    }
    return -1;
}

int Media::countChildren() { return children.size(); }

bool Media::appendChild(std::weak_ptr<Media> child) {
    if (childType == MediaType::NONE || childIndex(child) != -1) {
        return false;
    }
    shared_ptr<Media> new_child = child.lock();
    if (childType != MediaType::ANY && new_child->getType() != childType) {
        throw(WrongMediaType());
    }

    children.push_back(new_child);
    return true;
}

bool Media::appendParent(std::weak_ptr<Media> parent) {
    if (parentType == MediaType::NONE || parentIndex(parent) != -1) {
        return false;
    }
    shared_ptr<Media> new_parent = parent.lock();
    if (new_parent->getType() != parentType) {
        throw(WrongMediaType());
    }

    parents.push_back(new_parent);
    return true;
}

QIcon Media::getPreview() { return QIcon(QPixmap::fromImage(icon)); }

std::vector<int> Media::getChildIds() {
    vector<int> out;
    for (auto& child : children) {
        out.push_back(child->getId());
    }
    return out;
}

std::vector<int> Media::getSiblingIds() {
    vector<int> out;

    for (auto parent : parents) {
        for (int child : parent->getChildIds()) {
            out.push_back(child);
        }
    }
    return out;
}
