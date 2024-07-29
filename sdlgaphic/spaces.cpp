#include<unordered_map>
#include<initializer_list>
#include<vector>
#include<functional>
struct Actor;
using ID = int;
enum class Type{ Center = 0,One,Two,Three,Four,Five,Six,Seven,Eight};
struct Cell{
    using Actors = std::unordered_map<ID,Actor*>;
    using Container = std::vector<Actor*>;
    Actors actors;
    bool get(Container& c){
        for(auto e:actors)c.push_back(e.second);
        return actors.empty();
    }

};
static const float max_uint64 = static_cast<float>(static_cast<uint64_t>(0xffffffffffffffffLL));
struct Space{
    using Cells = std::unordered_map<uint64_t,Cell*>;
    Cells cells;
    Space(bool zero = false):cells{}{}
};
struct Position{
    float x,y,z;
};
struct MoveComp{
    Position& pre;
    Position& post;
    ID id;
};
struct Spaces{
    using Container = std::vector<Actor*>;
    using Sep = std::unordered_map<Type,Space>;
    using MoveContainer = std::vector<MoveComp*>;
    using List = std::initializer_list<std::pair<const Type,Space>>;
    Sep seps;
    Spaces(List&& s):seps(s){}
    // virtual bool get(const Position& pos,float radius,Container& c) = 0;
    // virtual void move(MoveContainer& c) = 0;
};
// 【a,b)
Spaces s{
    {Type::Center,Space(true)}
    ,{Type::One,Space()}
    ,{Type::Two,Space()}
    ,{Type::Three,Space()}
    ,{Type::Four,Space()}
    ,{Type::Five,Space()}
    ,{Type::Six,Space()}
    ,{Type::Seven,Space()}
    ,{Type::Eight,Space()}
};


struct Camera;
struct Light;
struct RenderItem;
struct Renderer;
struct Terrain;
using Lights = std::vector<Light*>;
using Items = std::vector<RenderItem*>;
using Renderers = std::vector<Renderer*>;
struct RenderContext{
    Lights lights;
    Items items;
    Renderers renderers;
    Camera* camera;
    Terrain* terrain;
};

struct Resource;
enum class Res{

};
struct ResHolder{
    Res res;
    int ref;
    Resource* p;
};
using AllRes = std::unordered_map<Res,ResHolder*>;
struct Loader;
using LoadCallback = std::function<void(Resource*)>;
struct ResourceManager{
    Loader* loader;
    void require(Res id,LoadCallback cb){
        auto itr = res.find(id);
        if(res.end()!=itr){
            cb(itr->second->p);
        }else{
            load(id);
            cb(res.find(id)->second->p);
        }
    }
    void load(Res id){
        
    }
    AllRes res;
};


struct Config;
struct ConfigOption;
struct Network;

enum class GameElementType{
    COLLIDER
    ,MOVE
    ,RENDER
    ,RENDER_TICK
};

struct Target{
    float yAxisRot;//up always Y axis,when value is 0,face to Z axis,
    float x,y,z;//position on the ground
    float xAABB,yAABB,zAABB;//must positive
    float cx(){return (x+xAABB)/2;}
    float cz(){return (z+zAABB)/2;}
    float cy(){return (y+yAABB)/2;}
    float xcg(){return cx();}//x of center of ground
    float zcg(){return cz();}//z of center of ground
};

struct AABB{
    float xmin,ymin,zmin;
    float xmax,ymax,zmax;
};
using OrigionAABB = AABB;
struct CenterTarget{
    float yAxisRot;//up always Y axis,when value is 0,face to Z axis,
    float x,y,z;//position of the center of the target
    OrigionAABB oaabb;
    AABB currentAABB(){
        const auto pi = 3.14159f;
        const auto xlen = oaabb.xmax-oaabb.xmin;
        const auto zlen = oaabb.zmax-oaabb.zmin;
        const auto px = xlen/2;
        const auto pz = zlen/2;
        const auto theta = yAxisRot-((int)(yAxisRot/(2*pi))*2*pi);
        const auto ct = cosf(theta);
        const auto st = sinf(theta);
        const auto new_z = ct*pz-st*px;
        const auto new_x = ct*px+st*pz;
        const auto xabs = abs(new_x);
        const auto zabs = abs(new_z);
        return AABB{x-xabs,oaabb.ymin+y,z-zabs,x+xabs,oaabb.ymax+y,z+zabs};
    }
};

struct Camera;
struct CameraTarget;
struct CameraTargetController{
    Camera* camera;
    CameraTarget* target;
    void onTargetMoveFront(){}
    void onTargetMoveBack(){}
    void onTargetMoveLeft(){}
    void onTargetMoveRight(){}
    void onMouseMove(){}
    void onMouseButtonAction(int id,int action){}
};

int main(){
    return 0;
}