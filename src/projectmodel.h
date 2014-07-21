#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QList>
#include <QImage>
#include <QMap>
#include <QString>
#include <QPoint>
#include <QJsonObject>
#include <QUuid>

static const int MAX_PIVOTS = 4;
static const int MAX_CHILDREN = 4;
struct Part;
struct Composite;

enum class AssetType {
  Part = 0,
  Composite = 1,
  Folder = 2,
};

// TODO: Convert my v1 save files
#define PROJECT_SAVE_FILE_VERSION 2

struct AssetRef {
    QUuid uuid;
};

bool operator==(const AssetRef& a, const AssetRef& b);
bool operator<(const AssetRef& a, const AssetRef& b);

// using AssetRef = QUuid;

// Global access
class ProjectModel;
ProjectModel* PM();

/**
 * ProjectModel stores the static data of a project.
 * It is modified by QCommands supporting undo/redo (see http://qt-project.org/doc/qt-5.0/qtdoc/qundo.html )
 * Can access the global instance with PM()
 */
class ProjectModel
{
public:
    explicit ProjectModel();
    ~ProjectModel();
    static ProjectModel* Instance();

    AssetRef createAssetRef();

    // globally accessible assets of the model
    QString fileName;
    // QMap<QString,Part*> parts;
    // QMap<QString,Composite*> composites;

    // TODO: Access assets
    Part* getPart(const AssetRef& ref);
    bool hasPart(const AssetRef& ref);
    Composite* getComposite(const AssetRef& ref);
    bool hasComposite(const AssetRef& ref);

    // @deprecated because assets are referred to with the assetref/uuid
    Part* getPart(const QString& name); // NB: Returns the first part with this name, but may have similar named assets
    bool hasPart(const QString& name);
    Composite* getComposite(const QString& name); // NB: Returns the first part with this name, but may have similar named assets
    bool hasComposite(const QString& name);

    QMap<AssetRef, Part*> parts;
    QMap<AssetRef, Composite*> composites;

    // Change the project model
    void clear();
    bool load(const QString& fileName);
    bool save(const QString& fileName);

protected:
    static void JsonToPart(const QJsonObject& obj, const QMap<QString,QImage*>& imageMap, Part* part);
    static void PartToJson(const QString& name, const Part& part, QJsonObject* obj, QMap<QString,QImage*>* imageMap);
    static void CompositeToJson(const QString& name, const Composite& comp, QJsonObject* obj);
    static void JsonToComposite(const QJsonObject& obj, Composite* comp);
};


struct Asset {
  AssetRef ref;
  QString name;
};

struct Part: public Asset {
    ~Part();

    struct Mode {
        int width, height;
        int numFrames;
        int numPivots;
        int framesPerSecond;
        // bool looping; // we know whether something is looping in the driver that runs it
        QList<QImage*> images; // test: QImage img(16,16); img.fill(QColor(255,0,0));
        QList<QPoint> anchor;
        QList<QPoint> pivots[MAX_PIVOTS];        
    };

    QMap<QString,Mode> modes; // fourcc->mode
    QString properties;
};

struct Composite: public Asset {
    struct Child {
        QString part; // TODO: Ref should be a UUID

        int index; // in children
        int parent; // index of parent
        int parentPivot; // pivot index connected to
        int z; // z-order (comp is drawn from lowest z to highest z)
        QList<int> children; // list of children (indices)
    };

    int root;
    QMap<QString,Child> childrenMap;
    QList<QString> children;
    QString properties;
};


#endif // PROJECTMODEL_H