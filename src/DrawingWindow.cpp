#include<iostream>
#include<algorithm>
#include<math.h>
#include"DrawingWindow.h"
#include"Line.h"
#include"Circle.h"
#include"Triangle.h"
#include"Polygon.h"
#include"Arc.h"
#include"CurvedLine.h"

void DrawingWindow::draw_grid(){
    for(int i = pan_offset.x; i < window_size.x; i += cell_size.x)
        DrawLine(i, 0, i, window_size.y, {30, 30, 30, 255});
    for(int i = pan_offset.x; i > 0; i -= cell_size.x)
        DrawLine(i, 0, i, window_size.y, {30, 30, 30, 255});
    for(int i = pan_offset.y; i < window_size.y; i += cell_size.y)
        DrawLine(0, i, window_size.x, i, {30, 30, 30, 255});
    for(int i = pan_offset.y; i > 0; i -= cell_size.y)
        DrawLine(0, i, window_size.x, i, {30, 30, 30, 255});
    DrawLine(0, pan_offset.y, window_size.x, pan_offset.y, GRAY);
    DrawLine(pan_offset.x, 0, pan_offset.x, window_size.y, GRAY);
}

void DrawingWindow::draw_cursor(){
    DrawCircleV(get_mouse_window_rounded(), 10, {0, 220, 0, 100});
}

void DrawingWindow::mouse_input(){
    mouse_pos = GetMousePosition();
    Vector2 current_mouse_grid_pos = round_window_to_grid(mouse_pos);
    if(current_vertex != nullptr){
        current_vertex->x = current_mouse_grid_pos.x;
        current_vertex->y = current_mouse_grid_pos.y;
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(!repositioning_vertex)
                current_vertex = current_vertex->get_parent()->add_vertex(current_mouse_grid_pos);
            else{
                current_vertex = nullptr;
                repositioning_vertex = false;
            }
        }
    }else if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        current_vertex = select_vertex(current_mouse_grid_pos);
        repositioning_vertex = true;
    }
    if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)){
        Vector2 mouse_change = {prev_mouse_pos.x - mouse_pos.x, prev_mouse_pos.y - mouse_pos.y};
        pan_offset.x -= mouse_change.x;
        pan_offset.y -= mouse_change.y;
    }
    float wheel_move = GetMouseWheelMove();
    if(wheel_move != 0.0){
        Vector2 mouse_pos_before = window_to_grid(GetMousePosition());
        cell_size.x += wheel_move * 10;
        if(cell_size.x < 1)
            cell_size.x = 1;
        cell_size.y += wheel_move * 10;
        if(cell_size.y < 1)
            cell_size.y = 1;
        Vector2 mouse_pos_after = window_to_grid(GetMousePosition());
        pan_offset = grid_to_window({mouse_pos_after.x - mouse_pos_before.x, mouse_pos_after.y - mouse_pos_before.y});
    }
    prev_mouse_pos = mouse_pos;
}

void DrawingWindow::keyboard_input(){
    if(current_vertex == nullptr){
        if(IsKeyPressed(KEY_L))
            add_new_shape<Line>();
        else if(IsKeyPressed(KEY_C))
            add_new_shape<Circle>();
        else if(IsKeyPressed(KEY_T))
            add_new_shape<Triangle>();
        else if(IsKeyPressed(KEY_P))
            add_new_shape<Polygon>();
        else if(IsKeyPressed(KEY_A))
            add_new_shape<Arc>();
        else if(IsKeyPressed(KEY_S))
            add_new_shape<CurvedLine>();
    }else{
        if(IsKeyPressed(KEY_ESCAPE)){
            current_vertex = nullptr;
            repositioning_vertex = false;
        }else if (IsKeyPressed(KEY_DELETE)){
            remove_shape(current_vertex->get_parent());
            repositioning_vertex = false;
        }else if (IsKeyPressed(KEY_BACKSPACE)){
            remove_vertex(current_vertex);
            current_vertex = nullptr;
            repositioning_vertex = false;
        }
    }
}

template<class T>
void DrawingWindow::add_new_shape(){
    repositioning_vertex = false;
    Vector2 pos = round_window_to_grid(GetMousePosition());
    shapes.push_back(new T());
    shapes[shapes.size() - 1]->add_vertex(pos);
    current_vertex = shapes[shapes.size() - 1]->add_vertex(pos);
}

void DrawingWindow::remove_shape(Shape* shape){
    current_vertex = nullptr;
    shapes.erase(std::remove(shapes.begin(), shapes.end(), shape));
    delete shape;
}

void DrawingWindow::remove_vertex(Vertex* vertex){
    vertex->remove();
    if(vertex->parent->vertices.size() == 0)
        remove_shape(vertex->parent);
    delete vertex;
}

Vertex* DrawingWindow::select_vertex(Vector2 pos){
    for(Shape* shape: shapes)
        for(Vertex* vertex: shape->vertices){
            if(vertex->x == pos.x && vertex->y == pos.y)
                return vertex;
        }
    return nullptr;
}

Vector2 DrawingWindow::get_mouse_window_rounded(){
    return grid_to_window(round_window_to_grid(GetMousePosition()));
}

Vector2 DrawingWindow::grid_to_window(Vector2 pos){
    return {
        pos.x * cell_size.x + pan_offset.x,
        pos.y * cell_size.y + pan_offset.y,
    };
}

Vector2 DrawingWindow::window_to_grid(Vector2 pos){
    return {
        (pos.x - pan_offset.x) / cell_size.x,
        (pos.y - pan_offset.y) / cell_size.y,
    };
}

Vector2 DrawingWindow::round_window_to_grid(Vector2 pos){
    Vector2 grid_pos = window_to_grid(pos);
    return {
        roundf(grid_pos.x),
        roundf(grid_pos.y),
    };
}

DrawingWindow::DrawingWindow(Vector2 size):window_size(size){
    pan_offset = {size.x / 2, size.y / 2};
}

void DrawingWindow::run(){
    InitWindow(window_size.x, window_size.y, "Drawing Window");
    SetExitKey(KEY_Q);
    SetTargetFPS(30);
    while(!WindowShouldClose()){
        mouse_input();
        keyboard_input();
        BeginDrawing();
        ClearBackground(BLACK);
        draw_grid();
        draw_cursor();
        for(auto shape: shapes)
            shape->draw(this);
        EndDrawing();
    }
    CloseWindow();
}

DrawingWindow::~DrawingWindow(){
    for(auto shape: shapes)
        delete shape;
    shapes.clear();
}
