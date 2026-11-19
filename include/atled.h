#define RECT_NAME_LEN 64


typedef struct{
    Rectangle rectangle;
    char name[RECT_NAME_LEN];
    size_t name_len;
}TexRec;


typedef enum{
    MD_DRAW,
    MD_RENAME,
}EditorMode;

typedef struct {
    char * str;
    size_t len;
    size_t capacity;
}StringBuild;



typedef struct{
    Texture file_tex;
    Texture snap_tex;
}BarTextures;


typedef struct{

    bool is_texture_loaded;
    bool is_rec_selected;
    bool is_snap_menu_opened;
    bool is_help_menu_opened;

    
    Texture2D texture;
    float scale;

    Vector2 first_point;
    Vector2 second_point;


    EditorMode mode;


    Vector2 texture_position;

    Vector2 grid_size;
    Vector2 grid_offset;

    bool snap;

    size_t rec_byte_capacity;
    size_t rec_byte_count;
    size_t rec_count;



    char * output;
    char * prefix;

    
    size_t selected_rec;

    BarTextures bar_textures;

    TexRec * rectangles;
    StringBuild sb;

}Editor;
