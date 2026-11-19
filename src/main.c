#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "raylib.h"
#include "atled.h"
#include "trec.h"



void CreateStringBuild(Editor * editor){
    editor->sb.len = 0;
    editor->sb.capacity = 32;
    editor->sb.str = malloc(sizeof(char) * editor->sb.capacity);
}


void SbAddRec(Editor * editor,size_t rec_id){
    Rectangle tex_rec = editor->rectangles[rec_id].rectangle;
    const char * string = TextFormat("#define %s_%s (Rectangle){%.2f,%.2f,%.2f,%.2f}\n", editor->prefix, editor->rectangles[rec_id].name, tex_rec.x,tex_rec.y,tex_rec.width,tex_rec.height);
    printf("NAME %zu: %s\n", rec_id, editor->rectangles[rec_id].name);
    printf("GENERATED: %s\n",string);
    while(editor->sb.capacity <= editor->sb.len + strlen(string)){
	editor->sb.capacity *=2;
    }
    editor->sb.str = realloc(editor->sb.str, sizeof(char)*editor->sb.capacity);


    for (size_t i = 0;i < strlen(string); i++){
	editor->sb.str[editor->sb.len + i] = string[i];
    }
    editor->sb.len += strlen(string);
}


void ReadRects(){

}


Rectangle ModifyRectangle(Editor editor,Rectangle rec, float scale){
    rec.x = scale*rec.x + editor.texture_position.x;
    rec.y = scale*rec.y + editor.texture_position.y;
    rec.width *= scale;
    rec.height *= scale;
    return rec;
}

void LoadTextures(BarTextures * bar_textures){
    bar_textures->file_tex = LoadTexture("./textures/file.png");
    bar_textures->snap_tex = LoadTexture("./textures/grid.png");

}


void InitEditor(Editor * editor){

    editor->is_texture_loaded = false;
    editor->is_help_menu_opened = false;


    
    editor->scale = 3.0f;
    editor->texture_position = (Vector2){0,0};

    

    editor->snap = false;
    editor->rec_byte_capacity = sizeof(TexRec)*4;
    editor->rec_count = 0;

    CreateStringBuild(editor);

    editor->output = "./output/trec.h";
    editor->prefix = "TREC";

    editor->mode = MD_DRAW;

    LoadTextures(&editor->bar_textures);

    editor->is_rec_selected = false;
    editor->selected_rec = 0;

    editor->grid_size.x = 8;
    editor->grid_size.y = 8;
    editor->grid_offset.x = 0;
    editor->grid_offset.y = 0;
    editor->rectangles = malloc(editor->rec_byte_capacity);

}


void DrawBar(Editor * editor){
    DrawRectangle(0,0,GetRenderWidth(),30,BLACK);
    DrawLine(0,30,GetRenderWidth(),30,WHITE);

    DrawTextureEx(editor->bar_textures.file_tex,(Vector2){14,0},0.0f,2.0f,WHITE);
    DrawLine(60,0,60,30,WHITE);


    if (editor->snap){
	DrawRectangle(60,0,60,30,GRAY);

    }
    if (editor->is_rec_selected && editor->mode == MD_RENAME){
	DrawText(TextFormat("NAME: %s", editor->rectangles[editor->selected_rec].name), 200,5,20,WHITE);
    }

    DrawTextureEx(editor->bar_textures.snap_tex,(Vector2){74,0},0.0f,2.0f,WHITE);
    DrawLine(120,0,120,30,WHITE);
}

void FreeEditor(Editor * editor){
    free(editor->rectangles);
}

int main(){
    InitWindow(1280,720, "Atled");
    SetWindowIcon(LoadImage("./atled.png"));

    Editor editor = {0};
    InitEditor(&editor);


    Rectangle current_rectangle = {0,0,0,0};
    editor.texture_position = (Vector2){GetScreenWidth()*0.5-editor.texture.width*0.5*editor.scale,GetScreenHeight()*0.5-editor.texture.height*0.5*editor.scale};

    while (!WindowShouldClose()){
	BeginDrawing();
	ClearBackground((Color){25,25,25,255});




	float mouse_wheel_move = GetMouseWheelMove();

	if (mouse_wheel_move != 0.0f){
	    editor.scale += mouse_wheel_move;

	    bool stop = false;
	    if (editor.scale > 10 || editor.scale < 2.0){
		stop = true;
	    }
	    editor.scale = editor.scale <= 2.0f ? 2.0f : editor.scale;
	    editor.scale = editor.scale >= 10.0f ? 10.0f : editor.scale;

	    if(!stop){
		editor.texture_position.x += (editor.texture.width*editor.scale - editor.texture.width*(editor.scale+mouse_wheel_move))*0.5 - (GetMouseX()-GetScreenWidth()*0.5)/editor.scale;
		editor.texture_position.y += (editor.texture.height*editor.scale - editor.texture.height*(editor.scale+mouse_wheel_move))*0.5  - (GetMouseY()-GetScreenHeight()*0.5)/editor.scale;
	    }



	}
	if (IsKeyPressed(KEY_F4)){
	    ToggleFullscreen();
	    editor.texture_position = (Vector2){GetScreenWidth()*0.5-editor.texture.width*0.5*editor.scale,GetScreenHeight()*0.5-editor.texture.height*0.5*editor.scale};
	}

	if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
	    editor.texture_position.x += GetMouseDelta().x;
	    editor.texture_position.y += GetMouseDelta().y;
	}


	int amount = IsKeyDown(KEY_LEFT_SHIFT) ? 5 : 1; // for gride move and resize
	
	
	if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
	    size_t prev_rec = editor.selected_rec;
	    bool selected = false;

	    editor.selected_rec = 0;
	    for (size_t i = 0; i < editor.rec_count; i++){
		if (CheckCollisionPointRec((Vector2){(GetMouseX()-editor.texture_position.x)/editor.scale,(GetMouseY()-editor.texture_position.y)/editor.scale},editor.rectangles[i].rectangle) && (prev_rec != i || !editor.is_rec_selected)){
		    editor.selected_rec = i;
		    selected = true;
		    break;
		}
	    }
	    editor.is_rec_selected = selected;

	}

	if (IsKeyDown(KEY_LEFT_CONTROL)){
	    if(IsKeyPressed(KEY_S)){
		editor.snap = !editor.snap;
	    }


	    if(IsKeyPressed(KEY_H)){
		editor.is_help_menu_opened = !editor.is_help_menu_opened;
	    }
	    
	    if (IsKeyPressed(KEY_G)){
		for (size_t i = 0; i < editor.rec_count; i++){
		    SbAddRec(&editor,i);
		}
		FILE * output_file = fopen(TextFormat("%s", editor.output),"w");
		int ret = fwrite(editor.sb.str,sizeof(char)*editor.sb.len,1, output_file);
		free(editor.sb.str);
		editor.sb.capacity = 32;
		editor.sb.str = malloc(sizeof(char) * editor.sb.capacity);
		editor.sb.len = 0;
		fclose(output_file);
	    }
	    if (editor.is_rec_selected){
		if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_D))){
		    for (size_t i = editor.selected_rec; i < editor.rec_count-1; i++){
			editor.rectangles[i] = editor.rectangles[i+1];
		    }
		    editor.is_rec_selected = false;
		    editor.selected_rec = 0;
		    editor.rec_count--;
		}

		if (IsKeyPressed(KEY_R)){
		    editor.mode = editor.mode == MD_RENAME ? MD_DRAW : MD_RENAME;
		}
	    }

	    if (IsKeyPressed(KEY_LEFT)){
		editor.grid_size.x = editor.grid_size.x > amount ? (editor.grid_size.x - amount) : amount;
	    }
	    if (IsKeyPressed(KEY_RIGHT)){
		editor.grid_size.x += amount;
	    }
	    if (IsKeyPressed(KEY_UP)){
		editor.grid_size.y = editor.grid_size.y > amount ? (editor.grid_size.y - amount) : amount;
	    }
	    if (IsKeyPressed(KEY_DOWN)){
		editor.grid_size.y += amount;
	    }
	    
	}
	else{
	    if (IsKeyPressed(KEY_LEFT)){
		editor.grid_offset.x -= amount;
	    }
	    if (IsKeyPressed(KEY_RIGHT)){
		editor.grid_offset.x += amount;
	    }
	    if (IsKeyPressed(KEY_UP)){
		editor.grid_offset.y -= amount;
	    }
	    if (IsKeyPressed(KEY_DOWN)){
		editor.grid_offset.y += amount;
	    }

	}





	switch (editor.mode){
	case MD_DRAW:
	    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
		editor.first_point = (Vector2){floor((GetMouseX()-editor.texture_position.x)/editor.scale), floor((GetMouseY()-editor.texture_position.y)/editor.scale)};

		
		current_rectangle.x = editor.snap ? floor(editor.first_point.x/editor.grid_size.x)*editor.grid_size.x + editor.grid_offset.x : editor.first_point.x;
		current_rectangle.y = editor.snap ? floor(editor.first_point.y/editor.grid_size.y)*editor.grid_size.y + editor.grid_offset.y : editor.first_point.y;
	    }
	    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
		editor.second_point = (Vector2){floor((GetMouseX()-editor.texture_position.x)/editor.scale), floor((GetMouseY()-editor.texture_position.y)/editor.scale)};

		Vector2 position = (Vector2){editor.second_point.x- editor.first_point.x,editor.second_point.y- editor.first_point.y};

		current_rectangle.width = editor.snap ? floor(position.x/editor.grid_size.x+1)*editor.grid_size.x : position.x;



		current_rectangle.height = editor.snap ? floor(position.y/editor.grid_size.y+1)*editor.grid_size.y : position.y;


		if(current_rectangle.width < 0){
		    current_rectangle.width = fabs(current_rectangle.width);

		    float position_x = editor.first_point.x - current_rectangle.width;
		    current_rectangle.x = editor.snap ?  floor(position_x/editor.grid_size.x+1)*editor.grid_size.x : position_x;
		}
		if(current_rectangle.height < 0){
		    current_rectangle.height = fabs(current_rectangle.height);

		    float position_y = editor.first_point.y - current_rectangle.height;

		    current_rectangle.y = editor.snap ?  floor(position_y/editor.grid_size.y+1)*editor.grid_size.y : position_y;

		}

	    }

	    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){

		if (current_rectangle.width != 0 && current_rectangle.height != 0){
		    editor.rectangles[editor.rec_count].rectangle = current_rectangle;
		    editor.rectangles[editor.rec_count].name_len = 0;
		    editor.rectangles[editor.rec_count].name[0] = '\0';
		    editor.rec_count++;

		    editor.rec_byte_count += sizeof(TexRec);

		    if(editor.rec_byte_count >= editor.rec_byte_capacity){
			editor.rec_byte_capacity *= 2;
			editor.rectangles = realloc(editor.rectangles, editor.rec_byte_capacity);
		    }
		}
		current_rectangle = (Rectangle){0,0,0,0};
	    }

	    
	    break;
	case MD_RENAME:
	    if (editor.is_rec_selected){
		if (IsKeyPressed(KEY_BACKSPACE) && editor.rectangles[editor.selected_rec].name_len >= 1){
		    editor.rectangles[editor.selected_rec].name_len--;
		    editor.rectangles[editor.selected_rec].name[editor.rectangles[editor.selected_rec].name_len] = '\0';
		}
		char key = GetCharPressed();

		if ((key >= 32 && key <= 125) && editor.rectangles[editor.selected_rec].name_len < RECT_NAME_LEN-1){
		    editor.rectangles[editor.selected_rec].name[editor.rectangles[editor.selected_rec].name_len] = key;
		    editor.rectangles[editor.selected_rec].name_len++;
		    editor.rectangles[editor.selected_rec].name[editor.rectangles[editor.selected_rec].name_len] = '\0';
		}
	    }
	    else{
		editor.mode = MD_DRAW;
	    }

	    break;
	}


	if (IsFileDropped()){
	    FilePathList files = LoadDroppedFiles();

	    if (strcmp(".h", GetFileExtension(files.paths[0])) == 0){

	    }

	    if (strcmp(".png", GetFileExtension(files.paths[0])) == 0){
		UnloadTexture(editor.texture);
		editor.texture = LoadTexture(files.paths[0]);

		if (IsTextureValid(editor.texture)){
		    editor.is_texture_loaded = true;
		}
	    }


	    UnloadDroppedFiles(files);
	}	

	

	DrawTextureEx(editor.texture, editor.texture_position, 0.0f, editor.scale,WHITE);
	
	for(int i = -1; i * editor.grid_size.x * editor.scale < editor.texture.width*editor.scale; i++){
	    float position = i * editor.grid_size.x * editor.scale + editor.grid_offset.x*editor.scale + editor.texture_position.x;
	    DrawLine(position ,0,position,GetScreenHeight(),(Color){128,128,128,128});
	}

	for(int i = -1; i * editor.grid_size.y * editor.scale < editor.texture.height*editor.scale; i++){
	    float position = i * editor.grid_size.y * editor.scale + editor.grid_offset.y*editor.scale + editor.texture_position.y;
	    DrawLine(0,position,GetScreenWidth(),position,(Color){128,128,128,128});
	}
	
	DrawRectangleLinesEx((Rectangle){editor.texture_position.x,editor.texture_position.y,editor.texture.width*editor.scale,editor.texture.height*editor.scale},1,GREEN);
	
	DrawRectangleLinesEx(ModifyRectangle(editor,current_rectangle,editor.scale),2.0f,RED);
	
	for(size_t i = 0; i < editor.rec_count; i++){
	    Color color = i == editor.selected_rec && editor.is_rec_selected ? BLUE : YELLOW;
	    DrawRectangleLinesEx(ModifyRectangle(editor,editor.rectangles[i].rectangle,editor.scale),2.0f,color);

	}
	
	if (!editor.is_texture_loaded){
	    const char * text = "DROP OR LOAD TEXTURE";
	    DrawText(text, GetRenderWidth()/2 - MeasureText(text,50)/2, GetRenderHeight()/2-25,50,WHITE );
	}

	DrawBar(&editor);


	DrawRectangle(0,31,180,120,(Color){0,0,0,160});
	DrawText(TextFormat("SNAP X: %.0f",editor.grid_offset.x),5,51,20,WHITE);
	DrawText(TextFormat("SNAP Y: %.0f",editor.grid_offset.y),5,71,20,WHITE);

	DrawText(TextFormat("SNAP SX: %.0f",editor.grid_size.x),5,91,20,WHITE);
	DrawText(TextFormat("SNAP SY: %.0f",editor.grid_size.y),5,111,20,WHITE);

	DrawLine(180,0,180,150,WHITE);
	DrawLine(0,150,180,150,WHITE);




	if (editor.is_help_menu_opened){
	    DrawText("RMB to select rect",5,GetRenderHeight()-20,20,WHITE);
	    DrawText("Ctrl + D to delete rect",5,GetRenderHeight()-40,20,WHITE);
	    DrawText("Ctrl + R to rename rect",5,GetRenderHeight()-60,20,WHITE);
	    DrawText("Ctrl + S for SNAP",5,GetRenderHeight()-80,20,WHITE);
	    DrawText("Arrows to move grid (shift for more)",5,GetRenderHeight()-100,20,WHITE);
	    DrawText("CTRL + Arrows to resize grid (shift for more)",5,GetRenderHeight()-120,20,WHITE);
	}
	else{
	    DrawText("Ctrl + H for help",5,GetRenderHeight()-20,20,WHITE);
	}
	
	EndDrawing();

    }
    FreeEditor(&editor);

}
