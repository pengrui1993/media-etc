#include<vector>
#include<cinttypes>
#include<memory>
#include<unordered_map>
#include<unordered_set>
class Object;
class Element;
class ElementFactory;
class Level;
class LevelFactory;
class World;
class PlayerState;
class GameState;
class GameMode;

enum class ObjectType:uint32_t{
    ACTOR
};
class Object{
public:
    virtual ObjectType type() = 0;
};

class Actor:public Object{
    virtual ObjectType type() override{
        return ObjectType::ACTOR;
    }
};
class Element{


private:
    Object*  item;
};

class Level{
using List = std::vector<Element>;

private:
    List elements;
};

class World{
public:
    Level& curLevel(){
        return *cur.get();
    }
private:
    std::shared_ptr<Level> cur;
    Level* current;
};

using ID = int;
using XIndex = int;
using ZIndex = int;
class Tony{
private:
    ID id;
    double angle;
    double x,z,y;
    int animation;
    double animationTime;
};
class DemoListener{
private:
    XIndex x;
    ZIndex z;
    std::vector<ID> objs;
};
class DemoLevel{
using List = std::vector<DemoListener>;
using Map = std::unordered_map<XIndex,std::unordered_map<ZIndex,DemoListener>>;
using Set = std::unordered_set<ID>;
public:
    void onInit();
    void onJoin(ID);
    void onLeave(ID);
    void onMove(ID);
    void onDestroy();
private:
    double minx,minz,maxx,maxz;
    int count;
    int size(){return 0;}
    List listeners;
    Map listenerMap;
    Set elementsId;
};