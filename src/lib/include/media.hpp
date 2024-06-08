#pragma once
#include <QIcon>
#include <QString>
#include <qpixmap.h>

namespace kedia {

enum MediaType {
    NONE, // Do not allow a child or parent, or has no content
    SERIES,
    SEASON,
    EPISODE,
    ANY // Allow any type as child or parent
};

QString MediaTypeToString(MediaType t);

class WrongMediaType : public std::exception {
  public:
    WrongMediaType() : std::exception(){};
    const char* what() const throw() {
        return "Incorrect media type for this function.";
    }
};
class Media {

  public:
    // Create the given media using the media id, the media type, and the
    // media's parent and child types. This is generally called through the
    // subclass's constructor.
    Media(int id_in, MediaType type_in, MediaType parent_type_in,
          MediaType child_type_in) {
        id = id_in;
        type = type_in;
        parentType = parent_type_in;
        childType = child_type_in;
    };

    // Mandatory deconstructor, does nothing.
    virtual ~Media(){};

    // Returns the title of the media.
    virtual QString getTitle() { return title; };
    // Returns the icon associated with the media.
    virtual QIcon getPreview();
    // Returns the media's icon as a QImage.
    virtual QImage getImage() { return icon; };

    // Sets the media's title.
    virtual void setTitle(QString title_in) { title = title_in; };
    // Sets the media's icon with a QImage.
    virtual void setPreview(QImage icon_in) { icon = icon_in; };
    // Returns a vector of pointers to the media's parents. If it has none,
    // returns an empty vector.
    virtual std::vector<std::weak_ptr<Media>> getChildren();
    // Returns a vector of pointers to the media's children. If it has none,
    // returns an empty vector.
    virtual std::vector<std::weak_ptr<Media>> getParents();
    virtual bool appendChild(std::weak_ptr<Media> child);
    virtual bool appendParent(std::weak_ptr<Media> parent);

    // Returns a list of all sibling ids.
    std::vector<int> getSiblingIds();
    // Returns a list of all child ids.
    std::vector<int> getChildIds();
    // Returns true if the given media pointer is a child of the media.
    virtual bool hasChild(std::weak_ptr<Media> child);
    // Returns the number of children.
    virtual int countChildren();
    // Returns the index of a given child.
    virtual int childIndex(std::weak_ptr<Media> child);
    // Returns the index of a given parent.
    virtual int parentIndex(std::weak_ptr<Media> parent);
    // Returns the given child, from their index.
    virtual std::weak_ptr<Media> getChild(int index);
    // Returns the given child, from their index. Returns the given parent, from
    // their index. By default this is just the first index, as most media types
    // have only one parent.
    virtual std::weak_ptr<Media> getParent(int index = 0);
    // Returns true if the media has any children.
    virtual bool hasChildren();

    // Returns the type of media.
    virtual MediaType getType() { return type; };
    // Returns the type of media that a parent of this media should be.
    virtual MediaType getParentType() { return parentType; };
    // Returns the type of media that a child of this media should be.
    virtual MediaType getChildType() { return childType; };

    // Returns this media's media_id.
    virtual int getId() const { return id; };

  protected:
    std::vector<std::shared_ptr<Media>> children;
    std::vector<std::shared_ptr<Media>> parents;
    MediaType type;
    MediaType parentType;
    MediaType childType;

    QString title;
    QImage icon;

  private:
    int id;
};

// Helper function that turns a vector of shared pointers into a vector of
// weak pointers.
template <typename type>
std::vector<std::weak_ptr<type>>
sharedToWeak(std::vector<std::shared_ptr<type>> vec_in) {
    std::vector<std::weak_ptr<type>> vec_out;
    for (auto ptr : vec_in) {
        vec_out.push_back(ptr);
    }
    return vec_out;
}
} // namespace kedia
