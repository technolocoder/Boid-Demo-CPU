#include <SDL2/SDL.h>
#include <random>
#include <iostream>

using namespace std;

struct vec2 {float x,y;};

float get_dist(const vec2 &a, const vec2 &b){ return sqrt(pow(a.x-b.x,2)+pow(a.y-b.y,2)); }
float get_magnitude(const vec2 &vec){ return sqrt((vec.x*vec.x)+(vec.y*vec.y)); }
vec2 normalize_vec(const vec2 &vec) {
    float mag = get_magnitude(vec);
    return {vec.x/mag,vec.y/mag};
}

random_device rd;
mt19937_64 engine(rd());
uniform_real_distribution<float> dist(-1,1);

int window_width = 1333, window_height = 700;

class boid{
public:
    boid() {}
    boid(vec2 _position){
        index = size_counter++;
        position = _position;

        velocity.x = dist(engine);
        velocity.y = dist(engine);
    }

    vec2 calc_acc(boid *boids){
        vec2 out {0,0},norm_avg = normalize_vec(velocity), pos_avg = position;
        int count = 1;
        for(int i = 0; i < index; ++i){
            float dist = get_dist(position,boids[i].position);
            if(dist < max_dist){
                ++count;
                vec2 diff {boids[i].position.x-position.x,boids[i].position.y-position.y};
                vec2 direction = normalize_vec(diff);
                vec2 bdir = normalize_vec(boids[i].velocity);
                norm_avg.x += bdir.x;
                norm_avg.y += bdir.y;
                pos_avg.x += boids[i].position.x;
                pos_avg.y += boids[i].position.y;
                float force = max((30.0-dist)/1000,0.0);

                out.x -= direction.x * force;
                out.y -= direction.y * force;
            }
        }

        for(int i = index+1; i < size_counter; ++i){
            float dist = get_dist(position,boids[i].position);
            if(dist < max_dist){
                ++count;
                vec2 diff {boids[i].position.x-position.x,boids[i].position.y-position.y};
                vec2 direction = normalize_vec(diff);
                vec2 bdir = normalize_vec(boids[i].velocity);
                norm_avg.x += bdir.x;
                norm_avg.y += bdir.y;
                pos_avg.x += boids[i].position.x;
                pos_avg.y += boids[i].position.y;
                float force = max((30.0-dist)/1000,0.0);

                out.x -= direction.x * force;
                out.y -= direction.y * force;
            }
        }
        norm_avg.x /= count;
        norm_avg.y /= count; 

        pos_avg.x /= count;
        pos_avg.y /= count;

        vec2 diff = {norm_avg.x-velocity.x,norm_avg.y-velocity.y};
        vec2 norm = normalize_vec(diff);
        out.x += norm.x*0.001;
        out.y += norm.y*0.001;

        vec2 pos_diff = {pos_avg.x-position.x,pos_avg.y-position.y};
        vec2 pos_norm = normalize_vec(pos_diff);

        out.x += norm.x * 0.001;
        out.y += norm.y * 0.001;
        return out;
    }

    void update_velocity(const vec2 &acc){
        velocity.x += acc.x;
        velocity.y += acc.y;
    }

    void update_position(){
        position.x += velocity.x;
        position.y += velocity.y;

        if(position.x > window_width){
            position.x = 0;
        }else if(position.x < 0){
            position.x = window_width;
        }  

        if(position.y > window_height){
            position.y = 0;
        }else if(position.y < 0){
            position.y = window_height;
        }
    }

    void render(SDL_Renderer *renderer){
        SDL_Rect rect;
        rect.x = position.x;
        rect.y = position.y;
        rect.w = 1;
        rect.h = 1;

        SDL_SetRenderDrawColor(renderer,0xFF,0xFF,0xFF,0xFF);
        SDL_RenderFillRect(renderer,&rect);
    } 
private:
    vec2 position ,velocity;
    int index;
    static int size_counter;
    static float max_dist;
};

int boid::size_counter = 0;
float boid::max_dist = 50;

int main(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        cerr << "Error Initializing SDL: " << SDL_GetError() << '\n';
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Boids",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,window_width,window_height,SDL_WINDOW_SHOWN);
    if(!window){
        cerr << "Error Creating Window: " << SDL_GetError() << '\n';
        SDL_Quit();
        return -1;
    }

    SDL_Event event;
    bool quit = false;

    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    int boid_count = 1000;
    boid *boids = (boid*)malloc(sizeof(boid)*boid_count);


    uniform_int_distribution<int> distx(0,window_width), disty(0,window_height);
    for(int i = 0; i < boid_count; ++i) boids[i] = boid({distx(engine),disty(engine)});
    unsigned long long timer = 0,reference = 0;
    while(!quit){
        while(SDL_PollEvent(&event)){   
            if(event.type == SDL_QUIT){
                quit = true;
                break;
            }else if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                }
            }
        }
        timer = SDL_GetTicks();
        vec2 acc[boid_count];
        if(timer-reference > 1000/60.0){
            reference = timer;
            SDL_SetRenderDrawColor(renderer,0,0,0,255);
            SDL_RenderClear(renderer);

            for(int i = 0; i < boid_count; ++i) acc[i] = boids[i].calc_acc(boids);
            for(int i = 0; i < boid_count; ++i){
                boids[i].update_velocity(acc[i]);
                boids[i].update_position();
                boids[i].render(renderer);
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}