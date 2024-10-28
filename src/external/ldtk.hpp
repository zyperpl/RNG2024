//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     Ldtk data = nlohmann::json::parse(jsonString);

#pragma once

#include <optional>
#include "json.hpp"

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json & j, const std::shared_ptr<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::shared_ptr<T> from_json(const json & j) {
            if (j.is_null()) return std::make_shared<T>(); else return std::make_shared<T>(j.get<T>());
        }
    };
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json & j, const std::optional<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::optional<T> from_json(const json & j) {
            if (j.is_null()) return std::make_optional<T>(); else return std::make_optional<T>(j.get<T>());
        }
    };
}
#endif

namespace ldtk {
    using nlohmann::json;

    #ifndef NLOHMANN_UNTYPED_ldtk_HELPER
    #define NLOHMANN_UNTYPED_ldtk_HELPER
    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }
    #endif

    #ifndef NLOHMANN_OPTIONAL_ldtk_HELPER
    #define NLOHMANN_OPTIONAL_ldtk_HELPER
    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(const json & j, const char * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(const json & j, std::string property) {
        return get_heap_optional<T>(j, property.data());
    }
    template <typename T>
    inline std::optional<T> get_stack_optional(const json & j, const char * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::optional<T>>();
        }
        return std::optional<T>();
    }

    template <typename T>
    inline std::optional<T> get_stack_optional(const json & j, std::string property) {
        return get_stack_optional<T>(j, property.data());
    }
    #endif

    /**
     * Possible values: `Manual`, `AfterLoad`, `BeforeSave`, `AfterSave`
     */
    enum class When : int16_t { AFTER_LOAD, AFTER_SAVE, BEFORE_SAVE, MANUAL };

    struct LdtkCustomCommand {
        std::string command;
        /**
         * Possible values: `Manual`, `AfterLoad`, `BeforeSave`, `AfterSave`
         */
        When when;
    };

    /**
     * Possible values: `Any`, `OnlySame`, `OnlyTags`, `OnlySpecificEntity`
     */
    enum class AllowedRefs : int16_t { ANY, ONLY_SAME, ONLY_SPECIFIC_ENTITY, ONLY_TAGS };

    /**
     * Possible values: `Hidden`, `ValueOnly`, `NameAndValue`, `EntityTile`, `LevelTile`,
     * `Points`, `PointStar`, `PointPath`, `PointPathLoop`, `RadiusPx`, `RadiusGrid`,
     * `ArrayCountWithLabel`, `ArrayCountNoLabel`, `RefLinkBetweenPivots`,
     * `RefLinkBetweenCenters`
     */
    enum class EditorDisplayMode : int16_t { ARRAY_COUNT_NO_LABEL, ARRAY_COUNT_WITH_LABEL, ENTITY_TILE, HIDDEN, LEVEL_TILE, NAME_AND_VALUE, POINTS, POINT_PATH, POINT_PATH_LOOP, POINT_STAR, RADIUS_GRID, RADIUS_PX, REF_LINK_BETWEEN_CENTERS, REF_LINK_BETWEEN_PIVOTS, VALUE_ONLY };

    /**
     * Possible values: `Above`, `Center`, `Beneath`
     */
    enum class EditorDisplayPos : int16_t { ABOVE, BENEATH, CENTER };

    /**
     * Possible values: `ZigZag`, `StraightArrow`, `CurvedArrow`, `ArrowsLine`, `DashedLine`
     */
    enum class EditorLinkStyle : int16_t { ARROWS_LINE, CURVED_ARROW, DASHED_LINE, STRAIGHT_ARROW, ZIG_ZAG };

    enum class TextLanguageMode : int16_t { LANG_C, LANG_HAXE, LANG_JS, LANG_JSON, LANG_LOG, LANG_LUA, LANG_MARKDOWN, LANG_PYTHON, LANG_RUBY, LANG_XML };

    /**
     * This section is mostly only intended for the LDtk editor app itself. You can safely
     * ignore it.
     */
    struct FieldDefinition {
        /**
         * Human readable value type. Possible values: `Int, Float, String, Bool, Color,
         * ExternEnum.XXX, LocalEnum.XXX, Point, FilePath`.<br/>  If the field is an array, this
         * field will look like `Array<...>` (eg. `Array<Int>`, `Array<Point>` etc.)<br/>  NOTE: if
         * you enable the advanced option **Use Multilines type**, you will have "*Multilines*"
         * instead of "*String*" when relevant.
         */
        std::string type;
        /**
         * Optional list of accepted file extensions for FilePath value type. Includes the dot:
         * `.ext`
         */
        std::optional<std::vector<std::string>> accept_file_types;
        /**
         * Possible values: `Any`, `OnlySame`, `OnlyTags`, `OnlySpecificEntity`
         */
        AllowedRefs allowed_refs;
        std::optional<int64_t> allowed_refs_entity_uid;
        std::vector<std::string> allowed_ref_tags;
        bool allow_out_of_level_ref;
        /**
         * Array max length
         */
        std::optional<int64_t> array_max_length;
        /**
         * Array min length
         */
        std::optional<int64_t> array_min_length;
        bool auto_chain_ref;
        /**
         * TRUE if the value can be null. For arrays, TRUE means it can contain null values
         * (exception: array of Points can't have null values).
         */
        bool can_be_null;
        /**
         * Default value if selected value is null or invalid.
         */
        nlohmann::json default_override;
        /**
         * User defined documentation for this field to provide help/tips to level designers about
         * accepted values.
         */
        std::optional<std::string> doc;
        bool editor_always_show;
        bool editor_cut_long_values;
        std::optional<std::string> editor_display_color;
        /**
         * Possible values: `Hidden`, `ValueOnly`, `NameAndValue`, `EntityTile`, `LevelTile`,
         * `Points`, `PointStar`, `PointPath`, `PointPathLoop`, `RadiusPx`, `RadiusGrid`,
         * `ArrayCountWithLabel`, `ArrayCountNoLabel`, `RefLinkBetweenPivots`,
         * `RefLinkBetweenCenters`
         */
        EditorDisplayMode editor_display_mode;
        /**
         * Possible values: `Above`, `Center`, `Beneath`
         */
        EditorDisplayPos editor_display_pos;
        double editor_display_scale;
        /**
         * Possible values: `ZigZag`, `StraightArrow`, `CurvedArrow`, `ArrowsLine`, `DashedLine`
         */
        EditorLinkStyle editor_link_style;
        bool editor_show_in_world;
        std::optional<std::string> editor_text_prefix;
        std::optional<std::string> editor_text_suffix;
        /**
         * If TRUE, the field value will be exported to the `toc` project JSON field. Only applies
         * to Entity fields.
         */
        bool export_to_toc;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * TRUE if the value is an array of multiple values
         */
        bool is_array;
        /**
         * Max limit for value, if applicable
         */
        std::optional<double> max;
        /**
         * Min limit for value, if applicable
         */
        std::optional<double> min;
        /**
         * Optional regular expression that needs to be matched to accept values. Expected format:
         * `/some_reg_ex/g`, with optional "i" flag.
         */
        std::optional<std::string> regex;
        /**
         * If enabled, this field will be searchable through LDtk command palette
         */
        bool searchable;
        bool symmetrical_ref;
        /**
         * Possible values: &lt;`null`&gt;, `LangPython`, `LangRuby`, `LangJS`, `LangLua`, `LangC`,
         * `LangHaxe`, `LangMarkdown`, `LangJson`, `LangXml`, `LangLog`
         */
        std::optional<TextLanguageMode> text_language_mode;
        /**
         * UID of the tileset used for a Tile
         */
        std::optional<int64_t> tileset_uid;
        /**
         * Internal enum representing the possible field types. Possible values: F_Int, F_Float,
         * F_String, F_Text, F_Bool, F_Color, F_Enum(...), F_Point, F_Path, F_EntityRef, F_Tile
         */
        std::string field_definition_type;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * If TRUE, the color associated with this field will override the Entity or Level default
         * color in the editor UI. For Enum fields, this would be the color associated to their
         * values.
         */
        bool use_for_smart_color;
    };

    /**
     * Possible values: `DiscardOldOnes`, `PreventAdding`, `MoveLastOne`
     */
    enum class LimitBehavior : int16_t { DISCARD_OLD_ONES, MOVE_LAST_ONE, PREVENT_ADDING };

    /**
     * If TRUE, the maxCount is a "per world" limit, if FALSE, it's a "per level". Possible
     * values: `PerLayer`, `PerLevel`, `PerWorld`
     */
    enum class LimitScope : int16_t { PER_LAYER, PER_LEVEL, PER_WORLD };

    /**
     * Possible values: `Rectangle`, `Ellipse`, `Tile`, `Cross`
     */
    enum class RenderMode : int16_t { CROSS, ELLIPSE, RECTANGLE, TILE };

    /**
     * This object represents a custom sub rectangle in a Tileset image.
     */
    struct TilesetRectangle {
        /**
         * Height in pixels
         */
        int64_t h;
        /**
         * UID of the tileset
         */
        int64_t tileset_uid;
        /**
         * Width in pixels
         */
        int64_t w;
        /**
         * X pixels coordinate of the top-left corner in the Tileset image
         */
        int64_t x;
        /**
         * Y pixels coordinate of the top-left corner in the Tileset image
         */
        int64_t y;
    };

    /**
     * An enum describing how the the Entity tile is rendered inside the Entity bounds. Possible
     * values: `Cover`, `FitInside`, `Repeat`, `Stretch`, `FullSizeCropped`,
     * `FullSizeUncropped`, `NineSlice`
     */
    enum class TileRenderMode : int16_t { COVER, FIT_INSIDE, FULL_SIZE_CROPPED, FULL_SIZE_UNCROPPED, NINE_SLICE, REPEAT, STRETCH };

    struct EntityDefinition {
        /**
         * If enabled, this entity is allowed to stay outside of the current level bounds
         */
        bool allow_out_of_bounds;
        /**
         * Base entity color
         */
        std::string color;
        /**
         * User defined documentation for this element to provide help/tips to level designers.
         */
        std::optional<std::string> doc;
        /**
         * If enabled, all instances of this entity will be listed in the project "Table of content"
         * object.
         */
        bool export_to_toc;
        /**
         * Array of field definitions
         */
        std::vector<FieldDefinition> field_defs;
        double fill_opacity;
        /**
         * Pixel height
         */
        int64_t height;
        bool hollow;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Only applies to entities resizable on both X/Y. If TRUE, the entity instance width/height
         * will keep the same aspect ratio as the definition.
         */
        bool keep_aspect_ratio;
        /**
         * Possible values: `DiscardOldOnes`, `PreventAdding`, `MoveLastOne`
         */
        LimitBehavior limit_behavior;
        /**
         * If TRUE, the maxCount is a "per world" limit, if FALSE, it's a "per level". Possible
         * values: `PerLayer`, `PerLevel`, `PerWorld`
         */
        LimitScope limit_scope;
        double line_opacity;
        /**
         * Max instances count
         */
        int64_t max_count;
        /**
         * Max pixel height (only applies if the entity is resizable on Y)
         */
        std::optional<int64_t> max_height;
        /**
         * Max pixel width (only applies if the entity is resizable on X)
         */
        std::optional<int64_t> max_width;
        /**
         * Min pixel height (only applies if the entity is resizable on Y)
         */
        std::optional<int64_t> min_height;
        /**
         * Min pixel width (only applies if the entity is resizable on X)
         */
        std::optional<int64_t> min_width;
        /**
         * An array of 4 dimensions for the up/right/down/left borders (in this order) when using
         * 9-slice mode for `tileRenderMode`.<br/>  If the tileRenderMode is not NineSlice, then
         * this array is empty.<br/>  See: https://en.wikipedia.org/wiki/9-slice_scaling
         */
        std::vector<int64_t> nine_slice_borders;
        /**
         * Pivot X coordinate (from 0 to 1.0)
         */
        double pivot_x;
        /**
         * Pivot Y coordinate (from 0 to 1.0)
         */
        double pivot_y;
        /**
         * Possible values: `Rectangle`, `Ellipse`, `Tile`, `Cross`
         */
        RenderMode render_mode;
        /**
         * If TRUE, the entity instances will be resizable horizontally
         */
        bool resizable_x;
        /**
         * If TRUE, the entity instances will be resizable vertically
         */
        bool resizable_y;
        /**
         * Display entity name in editor
         */
        bool show_name;
        /**
         * An array of strings that classifies this entity
         */
        std::vector<std::string> tags;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.2.0  Replaced
         * by: `tileRect`
         */
        std::optional<int64_t> tile_id;
        double tile_opacity;
        /**
         * An object representing a rectangle from an existing Tileset
         */
        std::optional<TilesetRectangle> tile_rect;
        /**
         * An enum describing how the the Entity tile is rendered inside the Entity bounds. Possible
         * values: `Cover`, `FitInside`, `Repeat`, `Stretch`, `FullSizeCropped`,
         * `FullSizeUncropped`, `NineSlice`
         */
        TileRenderMode tile_render_mode;
        /**
         * Tileset ID used for optional tile display
         */
        std::optional<int64_t> tileset_id;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * This tile overrides the one defined in `tileRect` in the UI
         */
        std::optional<TilesetRectangle> ui_tile_rect;
        /**
         * Pixel width
         */
        int64_t width;
    };

    struct EnumValueDefinition {
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.4.0  Replaced
         * by: `tileRect`
         */
        std::optional<std::vector<int64_t>> tile_src_rect;
        /**
         * Optional color
         */
        int64_t color;
        /**
         * Enum value
         */
        std::string id;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.4.0  Replaced
         * by: `tileRect`
         */
        std::optional<int64_t> tile_id;
        /**
         * Optional tileset rectangle to represents this value
         */
        std::optional<TilesetRectangle> tile_rect;
    };

    struct EnumDefinition {
        std::optional<std::string> external_file_checksum;
        /**
         * Relative path to the external file providing this Enum
         */
        std::optional<std::string> external_rel_path;
        /**
         * Tileset UID if provided
         */
        std::optional<int64_t> icon_tileset_uid;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * An array of user-defined tags to organize the Enums
         */
        std::vector<std::string> tags;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * All possible enum values, with their optional Tile infos.
         */
        std::vector<EnumValueDefinition> values;
    };

    /**
     * Checker mode Possible values: `None`, `Horizontal`, `Vertical`
     */
    enum class Checker : int16_t { HORIZONTAL, NONE, VERTICAL };

    /**
     * Defines how tileIds array is used Possible values: `Single`, `Stamp`
     */
    enum class TileMode : int16_t { SINGLE, STAMP };

    /**
     * This complex section isn't meant to be used by game devs at all, as these rules are
     * completely resolved internally by the editor before any saving. You should just ignore
     * this part.
     */
    struct AutoLayerRuleDefinition {
        /**
         * If FALSE, the rule effect isn't applied, and no tiles are generated.
         */
        bool active;
        double alpha;
        /**
         * When TRUE, the rule will prevent other rules to be applied in the same cell if it matches
         * (TRUE by default).
         */
        bool break_on_match;
        /**
         * Chances for this rule to be applied (0 to 1)
         */
        double chance;
        /**
         * Checker mode Possible values: `None`, `Horizontal`, `Vertical`
         */
        Checker checker;
        /**
         * If TRUE, allow rule to be matched by flipping its pattern horizontally
         */
        bool flip_x;
        /**
         * If TRUE, allow rule to be matched by flipping its pattern vertically
         */
        bool flip_y;
        /**
         * If TRUE, then the rule should be re-evaluated by the editor at one point
         */
        bool invalidated;
        /**
         * Default IntGrid value when checking cells outside of level bounds
         */
        std::optional<int64_t> out_of_bounds_value;
        /**
         * Rule pattern (size x size)
         */
        std::vector<int64_t> pattern;
        /**
         * If TRUE, enable Perlin filtering to only apply rule on specific random area
         */
        bool perlin_active;
        double perlin_octaves;
        double perlin_scale;
        double perlin_seed;
        /**
         * X pivot of a tile stamp (0-1)
         */
        double pivot_x;
        /**
         * Y pivot of a tile stamp (0-1)
         */
        double pivot_y;
        /**
         * Pattern width & height. Should only be 1,3,5 or 7.
         */
        int64_t size;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.5.0  Replaced
         * by: `tileRectsIds`
         */
        std::optional<std::vector<int64_t>> tile_ids;
        /**
         * Defines how tileIds array is used Possible values: `Single`, `Stamp`
         */
        TileMode tile_mode;
        /**
         * Max random offset for X tile pos
         */
        int64_t tile_random_x_max;
        /**
         * Min random offset for X tile pos
         */
        int64_t tile_random_x_min;
        /**
         * Max random offset for Y tile pos
         */
        int64_t tile_random_y_max;
        /**
         * Min random offset for Y tile pos
         */
        int64_t tile_random_y_min;
        /**
         * Array containing all the possible tile IDs rectangles (picked randomly).
         */
        std::vector<std::vector<int64_t>> tile_rects_ids;
        /**
         * Tile X offset
         */
        int64_t tile_x_offset;
        /**
         * Tile Y offset
         */
        int64_t tile_y_offset;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * X cell coord modulo
         */
        int64_t x_modulo;
        /**
         * X cell start offset
         */
        int64_t x_offset;
        /**
         * Y cell coord modulo
         */
        int64_t y_modulo;
        /**
         * Y cell start offset
         */
        int64_t y_offset;
    };

    struct AutoLayerRuleGroup {
        bool active;
        int64_t biome_requirement_mode;
        /**
         * *This field was removed in 1.0.0 and should no longer be used.*
         */
        std::optional<bool> collapsed;
        std::optional<std::string> color;
        std::optional<TilesetRectangle> icon;
        bool is_optional;
        std::string name;
        std::vector<std::string> required_biome_values;
        std::vector<AutoLayerRuleDefinition> rules;
        int64_t uid;
        bool uses_wizard;
    };

    /**
     * IntGrid value definition
     */
    struct IntGridValueDefinition {
        std::string color;
        /**
         * Parent group identifier (0 if none)
         */
        int64_t group_uid;
        /**
         * User defined unique identifier
         */
        std::optional<std::string> identifier;
        std::optional<TilesetRectangle> tile;
        /**
         * The IntGrid value itself
         */
        int64_t value;
    };

    /**
     * IntGrid value group definition
     */
    struct IntGridValueGroupDefinition {
        /**
         * User defined color
         */
        std::optional<std::string> color;
        /**
         * User defined string identifier
         */
        std::optional<std::string> identifier;
        /**
         * Group unique ID
         */
        int64_t uid;
    };

    /**
     * Type of the layer as Haxe Enum Possible values: `IntGrid`, `Entities`, `Tiles`,
     * `AutoLayer`
     */
    enum class Type : int16_t { AUTO_LAYER, ENTITIES, INT_GRID, TILES };

    struct LayerDefinition {
        /**
         * Type of the layer (*IntGrid, Entities, Tiles or AutoLayer*)
         */
        std::string type;
        /**
         * Contains all the auto-layer rule definitions.
         */
        std::vector<AutoLayerRuleGroup> auto_rule_groups;
        std::optional<int64_t> auto_source_layer_def_uid;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.2.0  Replaced
         * by: `tilesetDefUid`
         */
        std::optional<int64_t> auto_tileset_def_uid;
        std::optional<int64_t> auto_tiles_killed_by_other_layer_uid;
        std::optional<int64_t> biome_field_uid;
        /**
         * Allow editor selections when the layer is not currently active.
         */
        bool can_select_when_inactive;
        /**
         * Opacity of the layer (0 to 1.0)
         */
        double display_opacity;
        /**
         * User defined documentation for this element to provide help/tips to level designers.
         */
        std::optional<std::string> doc;
        /**
         * An array of tags to forbid some Entities in this layer
         */
        std::vector<std::string> excluded_tags;
        /**
         * Width and height of the grid in pixels
         */
        int64_t grid_size;
        /**
         * Height of the optional "guide" grid in pixels
         */
        int64_t guide_grid_hei;
        /**
         * Width of the optional "guide" grid in pixels
         */
        int64_t guide_grid_wid;
        bool hide_fields_when_inactive;
        /**
         * Hide the layer from the list on the side of the editor view.
         */
        bool hide_in_list;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Alpha of this layer when it is not the active one.
         */
        double inactive_opacity;
        /**
         * An array that defines extra optional info for each IntGrid value.<br/>  WARNING: the
         * array order is not related to actual IntGrid values! As user can re-order IntGrid values
         * freely, you may value "2" before value "1" in this array.
         */
        std::vector<IntGridValueDefinition> int_grid_values;
        /**
         * Group informations for IntGrid values
         */
        std::vector<IntGridValueGroupDefinition> int_grid_values_groups;
        /**
         * Parallax horizontal factor (from -1 to 1, defaults to 0) which affects the scrolling
         * speed of this layer, creating a fake 3D (parallax) effect.
         */
        double parallax_factor_x;
        /**
         * Parallax vertical factor (from -1 to 1, defaults to 0) which affects the scrolling speed
         * of this layer, creating a fake 3D (parallax) effect.
         */
        double parallax_factor_y;
        /**
         * If true (default), a layer with a parallax factor will also be scaled up/down accordingly.
         */
        bool parallax_scaling;
        /**
         * X offset of the layer, in pixels (IMPORTANT: this should be added to the `LayerInstance`
         * optional offset)
         */
        int64_t px_offset_x;
        /**
         * Y offset of the layer, in pixels (IMPORTANT: this should be added to the `LayerInstance`
         * optional offset)
         */
        int64_t px_offset_y;
        /**
         * If TRUE, the content of this layer will be used when rendering levels in a simplified way
         * for the world view
         */
        bool render_in_world_view;
        /**
         * An array of tags to filter Entities that can be added to this layer
         */
        std::vector<std::string> required_tags;
        /**
         * If the tiles are smaller or larger than the layer grid, the pivot value will be used to
         * position the tile relatively its grid cell.
         */
        double tile_pivot_x;
        /**
         * If the tiles are smaller or larger than the layer grid, the pivot value will be used to
         * position the tile relatively its grid cell.
         */
        double tile_pivot_y;
        /**
         * Reference to the default Tileset UID being used by this layer definition.<br/>
         * **WARNING**: some layer *instances* might use a different tileset. So most of the time,
         * you should probably use the `__tilesetDefUid` value found in layer instances.<br/>  Note:
         * since version 1.0.0, the old `autoTilesetDefUid` was removed and merged into this value.
         */
        std::optional<int64_t> tileset_def_uid;
        /**
         * Type of the layer as Haxe Enum Possible values: `IntGrid`, `Entities`, `Tiles`,
         * `AutoLayer`
         */
        Type layer_definition_type;
        /**
         * User defined color for the UI
         */
        std::optional<std::string> ui_color;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * Display tags
         */
        std::vector<std::string> ui_filter_tags;
        /**
         * Asynchronous rendering option for large/complex layers
         */
        bool use_async_render;
    };

    /**
     * In a tileset definition, user defined meta-data of a tile.
     */
    struct TileCustomMetadata {
        std::string data;
        int64_t tile_id;
    };

    enum class EmbedAtlas : int16_t { LDTK_ICONS };

    /**
     * In a tileset definition, enum based tag infos
     */
    struct EnumTagValue {
        std::string enum_value_id;
        std::vector<int64_t> tile_ids;
    };

    /**
     * The `Tileset` definition is the most important part among project definitions. It
     * contains some extra informations about each integrated tileset. If you only had to parse
     * one definition section, that would be the one.
     */
    struct TilesetDefinition {
        /**
         * Grid-based height
         */
        int64_t c_hei;
        /**
         * Grid-based width
         */
        int64_t c_wid;
        /**
         * The following data is used internally for various optimizations. It's always synced with
         * source image changes.
         */
        std::optional<std::map<std::string, nlohmann::json>> cached_pixel_data;
        /**
         * An array of custom tile metadata
         */
        std::vector<TileCustomMetadata> custom_data;
        /**
         * If this value is set, then it means that this atlas uses an internal LDtk atlas image
         * instead of a loaded one. Possible values: &lt;`null`&gt;, `LdtkIcons`
         */
        std::optional<EmbedAtlas> embed_atlas;
        /**
         * Tileset tags using Enum values specified by `tagsSourceEnumId`. This array contains 1
         * element per Enum value, which contains an array of all Tile IDs that are tagged with it.
         */
        std::vector<EnumTagValue> enum_tags;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Distance in pixels from image borders
         */
        int64_t padding;
        /**
         * Image height in pixels
         */
        int64_t px_hei;
        /**
         * Image width in pixels
         */
        int64_t px_wid;
        /**
         * Path to the source file, relative to the current project JSON file<br/>  It can be null
         * if no image was provided, or when using an embed atlas.
         */
        std::optional<std::string> rel_path;
        /**
         * Array of group of tiles selections, only meant to be used in the editor
         */
        std::vector<std::map<std::string, nlohmann::json>> saved_selections;
        /**
         * Space in pixels between all tiles
         */
        int64_t spacing;
        /**
         * An array of user-defined tags to organize the Tilesets
         */
        std::vector<std::string> tags;
        /**
         * Optional Enum definition UID used for this tileset meta-data
         */
        std::optional<int64_t> tags_source_enum_uid;
        int64_t tile_grid_size;
        /**
         * Unique Intidentifier
         */
        int64_t uid;
    };

    /**
     * If you're writing your own LDtk importer, you should probably just ignore *most* stuff in
     * the `defs` section, as it contains data that are mostly important to the editor. To keep
     * you away from the `defs` section and avoid some unnecessary JSON parsing, important data
     * from definitions is often duplicated in fields prefixed with a double underscore (eg.
     * `__identifier` or `__type`).  The 2 only definition types you might need here are
     * **Tilesets** and **Enums**.
     *
     * A structure containing all the definitions of this project
     */
    struct Definitions {
        /**
         * All entities definitions, including their custom fields
         */
        std::vector<EntityDefinition> entities;
        /**
         * All internal enums
         */
        std::vector<EnumDefinition> enums;
        /**
         * Note: external enums are exactly the same as `enums`, except they have a `relPath` to
         * point to an external source file.
         */
        std::vector<EnumDefinition> external_enums;
        /**
         * All layer definitions
         */
        std::vector<LayerDefinition> layers;
        /**
         * All custom fields available to all levels.
         */
        std::vector<FieldDefinition> level_fields;
        /**
         * All tilesets
         */
        std::vector<TilesetDefinition> tilesets;
    };

    enum class Flag : int16_t { DISCARD_PRE_CSV_INT_GRID, EXPORT_OLD_TABLE_OF_CONTENT_DATA, EXPORT_PRE_CSV_INT_GRID_FORMAT, IGNORE_BACKUP_SUGGEST, MULTI_WORLDS, PREPEND_INDEX_TO_LEVEL_FILE_NAMES, USE_MULTILINES_TYPE };

    struct FieldInstance {
        /**
         * Field definition identifier
         */
        std::string identifier;
        /**
         * Optional TilesetRect used to display this field (this can be the field own Tile, or some
         * other Tile guessed from the value, like an Enum).
         */
        std::optional<TilesetRectangle> tile;
        /**
         * Type of the field, such as `Int`, `Float`, `String`, `Enum(my_enum_name)`, `Bool`,
         * etc.<br/>  NOTE: if you enable the advanced option **Use Multilines type**, you will have
         * "*Multilines*" instead of "*String*" when relevant.
         */
        std::string type;
        /**
         * Actual value of the field instance. The value type varies, depending on `__type`:<br/>
         * - For **classic types** (ie. Integer, Float, Boolean, String, Text and FilePath), you
         * just get the actual value with the expected type.<br/>   - For **Color**, the value is an
         * hexadecimal string using "#rrggbb" format.<br/>   - For **Enum**, the value is a String
         * representing the selected enum value.<br/>   - For **Point**, the value is a
         * [GridPoint](#ldtk-GridPoint) object.<br/>   - For **Tile**, the value is a
         * [TilesetRect](#ldtk-TilesetRect) object.<br/>   - For **EntityRef**, the value is an
         * [EntityReferenceInfos](#ldtk-EntityReferenceInfos) object.<br/><br/>  If the field is an
         * array, then this `__value` will also be a JSON array.
         */
        nlohmann::json value;
        /**
         * Reference of the **Field definition** UID
         */
        int64_t def_uid;
        /**
         * Editor internal raw values
         */
        std::vector<nlohmann::json> real_editor_values;
    };

    struct EntityInstance {
        /**
         * Grid-based coordinates (`[x,y]` format)
         */
        std::vector<int64_t> grid;
        /**
         * Entity definition identifier
         */
        std::string identifier;
        /**
         * Pivot coordinates  (`[x,y]` format, values are from 0 to 1) of the Entity
         */
        std::vector<double> pivot;
        /**
         * The entity "smart" color, guessed from either Entity definition, or one its field
         * instances.
         */
        std::string smart_color;
        /**
         * Array of tags defined in this Entity definition
         */
        std::vector<std::string> tags;
        /**
         * Optional TilesetRect used to display this entity (it could either be the default Entity
         * tile, or some tile provided by a field value, like an Enum).
         */
        std::optional<TilesetRectangle> tile;
        /**
         * X world coordinate in pixels. Only available in GridVania or Free world layouts.
         */
        std::optional<int64_t> world_x;
        /**
         * Y world coordinate in pixels Only available in GridVania or Free world layouts.
         */
        std::optional<int64_t> world_y;
        /**
         * Reference of the **Entity definition** UID
         */
        int64_t def_uid;
        /**
         * An array of all custom fields and their values.
         */
        std::vector<FieldInstance> field_instances;
        /**
         * Entity height in pixels. For non-resizable entities, it will be the same as Entity
         * definition.
         */
        int64_t height;
        /**
         * Unique instance identifier
         */
        std::string iid;
        /**
         * Pixel coordinates (`[x,y]` format) in current level coordinate space. Don't forget
         * optional layer offsets, if they exist!
         */
        std::vector<int64_t> px;
        /**
         * Entity width in pixels. For non-resizable entities, it will be the same as Entity
         * definition.
         */
        int64_t width;
    };

    /**
     * This object describes the "location" of an Entity instance in the project worlds.
     *
     * IID information of this instance
     */
    struct ReferenceToAnEntityInstance {
        /**
         * IID of the refered EntityInstance
         */
        std::string entity_iid;
        /**
         * IID of the LayerInstance containing the refered EntityInstance
         */
        std::string layer_iid;
        /**
         * IID of the Level containing the refered EntityInstance
         */
        std::string level_iid;
        /**
         * IID of the World containing the refered EntityInstance
         */
        std::string world_iid;
    };

    /**
     * This object is just a grid-based coordinate used in Field values.
     */
    struct GridPoint {
        /**
         * X grid-based coordinate
         */
        int64_t cx;
        /**
         * Y grid-based coordinate
         */
        int64_t cy;
    };

    /**
     * IntGrid value instance
     */
    struct IntGridValueInstance {
        /**
         * Coordinate ID in the layer grid
         */
        int64_t coord_id;
        /**
         * IntGrid value
         */
        int64_t v;
    };

    /**
     * This structure represents a single tile from a given Tileset.
     */
    struct TileInstance {
        /**
         * Alpha/opacity of the tile (0-1, defaults to 1)
         */
        double a;
        /**
         * Internal data used by the editor.<br/>  For auto-layer tiles: `[ruleId, coordId]`.<br/>
         * For tile-layer tiles: `[coordId]`.
         */
        std::vector<int64_t> d;
        /**
         * "Flip bits", a 2-bits integer to represent the mirror transformations of the tile.<br/>
         * - Bit 0 = X flip<br/>   - Bit 1 = Y flip<br/>   Examples: f=0 (no flip), f=1 (X flip
         * only), f=2 (Y flip only), f=3 (both flips)
         */
        int64_t f;
        /**
         * Pixel coordinates of the tile in the **layer** (`[x,y]` format). Don't forget optional
         * layer offsets, if they exist!
         */
        std::vector<int64_t> px;
        /**
         * Pixel coordinates of the tile in the **tileset** (`[x,y]` format)
         */
        std::vector<int64_t> src;
        /**
         * The *Tile ID* in the corresponding tileset.
         */
        int64_t t;
    };

    struct LayerInstance {
        /**
         * Grid-based height
         */
        int64_t c_hei;
        /**
         * Grid-based width
         */
        int64_t c_wid;
        /**
         * Grid size
         */
        int64_t grid_size;
        /**
         * Layer definition identifier
         */
        std::string identifier;
        /**
         * Layer opacity as Float [0-1]
         */
        double opacity;
        /**
         * Total layer X pixel offset, including both instance and definition offsets.
         */
        int64_t px_total_offset_x;
        /**
         * Total layer Y pixel offset, including both instance and definition offsets.
         */
        int64_t px_total_offset_y;
        /**
         * The definition UID of corresponding Tileset, if any.
         */
        std::optional<int64_t> tileset_def_uid;
        /**
         * The relative path to corresponding Tileset, if any.
         */
        std::optional<std::string> tileset_rel_path;
        /**
         * Layer type (possible values: IntGrid, Entities, Tiles or AutoLayer)
         */
        std::string type;
        /**
         * An array containing all tiles generated by Auto-layer rules. The array is already sorted
         * in display order (ie. 1st tile is beneath 2nd, which is beneath 3rd etc.).<br/><br/>
         * Note: if multiple tiles are stacked in the same cell as the result of different rules,
         * all tiles behind opaque ones will be discarded.
         */
        std::vector<TileInstance> auto_layer_tiles;
        std::vector<EntityInstance> entity_instances;
        std::vector<TileInstance> grid_tiles;
        /**
         * Unique layer instance identifier
         */
        std::string iid;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.0.0  Replaced
         * by: `intGridCsv`
         */
        std::optional<std::vector<IntGridValueInstance>> int_grid;
        /**
         * A list of all values in the IntGrid layer, stored in CSV format (Comma Separated
         * Values).<br/>  Order is from left to right, and top to bottom (ie. first row from left to
         * right, followed by second row, etc).<br/>  `0` means "empty cell" and IntGrid values
         * start at 1.<br/>  The array size is `__cWid` x `__cHei` cells.
         */
        std::vector<int64_t> int_grid_csv;
        /**
         * Reference the Layer definition UID
         */
        int64_t layer_def_uid;
        /**
         * Reference to the UID of the level containing this layer instance
         */
        int64_t level_id;
        /**
         * An Array containing the UIDs of optional rules that were enabled in this specific layer
         * instance.
         */
        std::vector<int64_t> optional_rules;
        /**
         * This layer can use another tileset by overriding the tileset UID here.
         */
        std::optional<int64_t> override_tileset_uid;
        /**
         * X offset in pixels to render this layer, usually 0 (IMPORTANT: this should be added to
         * the `LayerDef` optional offset, so you should probably prefer using `__pxTotalOffsetX`
         * which contains the total offset value)
         */
        int64_t px_offset_x;
        /**
         * Y offset in pixels to render this layer, usually 0 (IMPORTANT: this should be added to
         * the `LayerDef` optional offset, so you should probably prefer using `__pxTotalOffsetX`
         * which contains the total offset value)
         */
        int64_t px_offset_y;
        /**
         * Random seed used for Auto-Layers rendering
         */
        int64_t seed;
        /**
         * Layer instance visibility
         */
        bool visible;
    };

    /**
     * Level background image position info
     */
    struct LevelBackgroundPosition {
        /**
         * An array of 4 float values describing the cropped sub-rectangle of the displayed
         * background image. This cropping happens when original is larger than the level bounds.
         * Array format: `[ cropX, cropY, cropWidth, cropHeight ]`
         */
        std::vector<double> crop_rect;
        /**
         * An array containing the `[scaleX,scaleY]` values of the **cropped** background image,
         * depending on `bgPos` option.
         */
        std::vector<double> scale;
        /**
         * An array containing the `[x,y]` pixel coordinates of the top-left corner of the
         * **cropped** background image, depending on `bgPos` option.
         */
        std::vector<int64_t> top_left_px;
    };

    enum class BgPos : int16_t { CONTAIN, COVER, COVER_DIRTY, REPEAT, UNSCALED };

    /**
     * Nearby level info
     */
    struct NeighbourLevel {
        /**
         * A lowercase string tipping on the level location (`n`orth, `s`outh, `w`est,
         * `e`ast).<br/>  Since 1.4.0, this value can also be `<` (neighbour depth is lower), `>`
         * (neighbour depth is greater) or `o` (levels overlap and share the same world
         * depth).<br/>  Since 1.5.3, this value can also be `nw`,`ne`,`sw` or `se` for levels only
         * touching corners.
         */
        std::string dir;
        /**
         * Neighbour Instance Identifier
         */
        std::string level_iid;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 1.2.0  Replaced
         * by: `levelIid`
         */
        std::optional<int64_t> level_uid;
    };

    /**
     * This section contains all the level data. It can be found in 2 distinct forms, depending
     * on Project current settings:  - If "*Separate level files*" is **disabled** (default):
     * full level data is *embedded* inside the main Project JSON file, - If "*Separate level
     * files*" is **enabled**: level data is stored in *separate* standalone `.ldtkl` files (one
     * per level). In this case, the main Project JSON file will still contain most level data,
     * except heavy sections, like the `layerInstances` array (which will be null). The
     * `externalRelPath` string points to the `ldtkl` file.  A `ldtkl` file is just a JSON file
     * containing exactly what is described below.
     */
    struct Level {
        /**
         * Background color of the level (same as `bgColor`, except the default value is
         * automatically used here if its value is `null`)
         */
        std::string bg_color;
        /**
         * Position informations of the background image, if there is one.
         */
        std::optional<LevelBackgroundPosition> bg_pos;
        /**
         * An array listing all other levels touching this one on the world map. Since 1.4.0, this
         * includes levels that overlap in the same world layer, or in nearby world layers.<br/>
         * Only relevant for world layouts where level spatial positioning is manual (ie. GridVania,
         * Free). For Horizontal and Vertical layouts, this array is always empty.
         */
        std::vector<NeighbourLevel> neighbours;
        /**
         * The "guessed" color for this level in the editor, decided using either the background
         * color or an existing custom field.
         */
        std::string smart_color;
        /**
         * Background color of the level. If `null`, the project `defaultLevelBgColor` should be
         * used.
         */
        std::optional<std::string> level_bg_color;
        /**
         * Background image X pivot (0-1)
         */
        double bg_pivot_x;
        /**
         * Background image Y pivot (0-1)
         */
        double bg_pivot_y;
        /**
         * An enum defining the way the background image (if any) is positioned on the level. See
         * `__bgPos` for resulting position info. Possible values: &lt;`null`&gt;, `Unscaled`,
         * `Contain`, `Cover`, `CoverDirty`, `Repeat`
         */
        std::optional<BgPos> level_bg_pos;
        /**
         * The *optional* relative path to the level background image.
         */
        std::optional<std::string> bg_rel_path;
        /**
         * This value is not null if the project option "*Save levels separately*" is enabled. In
         * this case, this **relative** path points to the level Json file.
         */
        std::optional<std::string> external_rel_path;
        /**
         * An array containing this level custom field values.
         */
        std::vector<FieldInstance> field_instances;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Unique instance identifier
         */
        std::string iid;
        /**
         * An array containing all Layer instances. **IMPORTANT**: if the project option "*Save
         * levels separately*" is enabled, this field will be `null`.<br/>  This array is **sorted
         * in display order**: the 1st layer is the top-most and the last is behind.
         */
        std::optional<std::vector<LayerInstance>> layer_instances;
        /**
         * Height of the level in pixels
         */
        int64_t px_hei;
        /**
         * Width of the level in pixels
         */
        int64_t px_wid;
        /**
         * Unique Int identifier
         */
        int64_t uid;
        /**
         * If TRUE, the level identifier will always automatically use the naming pattern as defined
         * in `Project.levelNamePattern`. Becomes FALSE if the identifier is manually modified by
         * user.
         */
        bool use_auto_identifier;
        /**
         * Index that represents the "depth" of the level in the world. Default is 0, greater means
         * "above", lower means "below".<br/>  This value is mostly used for display only and is
         * intended to make stacking of levels easier to manage.
         */
        int64_t world_depth;
        /**
         * World X coordinate in pixels.<br/>  Only relevant for world layouts where level spatial
         * positioning is manual (ie. GridVania, Free). For Horizontal and Vertical layouts, the
         * value is always -1 here.
         */
        int64_t world_x;
        /**
         * World Y coordinate in pixels.<br/>  Only relevant for world layouts where level spatial
         * positioning is manual (ie. GridVania, Free). For Horizontal and Vertical layouts, the
         * value is always -1 here.
         */
        int64_t world_y;
    };

    struct LdtkTocInstanceData {
        /**
         * An object containing the values of all entity fields with the `exportToToc` option
         * enabled. This object typing depends on actual field value types.
         */
        nlohmann::json fields;
        int64_t hei_px;
        /**
         * IID information of this instance
         */
        ReferenceToAnEntityInstance iids;
        int64_t wid_px;
        int64_t world_x;
        int64_t world_y;
    };

    struct LdtkTableOfContentEntry {
        std::string identifier;
        /**
         * **WARNING**: this deprecated value will be *removed* completely on version 1.7.0+
         * Replaced by: `instancesData`
         */
        std::optional<std::vector<ReferenceToAnEntityInstance>> instances;
        std::vector<LdtkTocInstanceData> instances_data;
    };

    enum class WorldLayout : int16_t { FREE, GRID_VANIA, LINEAR_HORIZONTAL, LINEAR_VERTICAL };

    /**
     * **IMPORTANT**: this type is available as a preview. You can rely on it to update your
     * importers, for when it will be officially available.  A World contains multiple levels,
     * and it has its own layout settings.
     */
    struct World {
        /**
         * Default new level height
         */
        int64_t default_level_height;
        /**
         * Default new level width
         */
        int64_t default_level_width;
        /**
         * User defined unique identifier
         */
        std::string identifier;
        /**
         * Unique instance identifer
         */
        std::string iid;
        /**
         * All levels from this world. The order of this array is only relevant in
         * `LinearHorizontal` and `linearVertical` world layouts (see `worldLayout` value).
         * Otherwise, you should refer to the `worldX`,`worldY` coordinates of each Level.
         */
        std::vector<Level> levels;
        /**
         * Height of the world grid in pixels.
         */
        int64_t world_grid_height;
        /**
         * Width of the world grid in pixels.
         */
        int64_t world_grid_width;
        /**
         * An enum that describes how levels are organized in this project (ie. linearly or in a 2D
         * space). Possible values: `Free`, `GridVania`, `LinearHorizontal`, `LinearVertical`, `null`
         */
        std::optional<WorldLayout> world_layout;
    };

    /**
     * This object is not actually used by LDtk. It ONLY exists to force explicit references to
     * all types, to make sure QuickType finds them and integrate all of them. Otherwise,
     * Quicktype will drop types that are not explicitely used.
     */
    struct ForcedRefs {
        std::optional<AutoLayerRuleGroup> auto_layer_rule_group;
        std::optional<AutoLayerRuleDefinition> auto_rule_def;
        std::optional<LdtkCustomCommand> custom_command;
        std::optional<Definitions> definitions;
        std::optional<EntityDefinition> entity_def;
        std::optional<EntityInstance> entity_instance;
        std::optional<ReferenceToAnEntityInstance> entity_reference_infos;
        std::optional<EnumDefinition> enum_def;
        std::optional<EnumValueDefinition> enum_def_values;
        std::optional<EnumTagValue> enum_tag_value;
        std::optional<FieldDefinition> field_def;
        std::optional<FieldInstance> field_instance;
        std::optional<GridPoint> grid_point;
        std::optional<IntGridValueDefinition> int_grid_value_def;
        std::optional<IntGridValueGroupDefinition> int_grid_value_group_def;
        std::optional<IntGridValueInstance> int_grid_value_instance;
        std::optional<LayerDefinition> layer_def;
        std::optional<LayerInstance> layer_instance;
        std::optional<Level> level;
        std::optional<LevelBackgroundPosition> level_bg_pos_infos;
        std::optional<NeighbourLevel> neighbour_level;
        std::optional<LdtkTableOfContentEntry> table_of_content_entry;
        std::optional<TileInstance> tile;
        std::optional<TileCustomMetadata> tile_custom_metadata;
        std::optional<TilesetDefinition> tileset_def;
        std::optional<TilesetRectangle> tileset_rect;
        std::optional<LdtkTocInstanceData> toc_instance_data;
        std::optional<World> world;
    };

    /**
     * Naming convention for Identifiers (first-letter uppercase, full uppercase etc.) Possible
     * values: `Capitalize`, `Uppercase`, `Lowercase`, `Free`
     */
    enum class IdentifierStyle : int16_t { CAPITALIZE, FREE, LOWERCASE, UPPERCASE };

    /**
     * "Image export" option when saving project. Possible values: `None`, `OneImagePerLayer`,
     * `OneImagePerLevel`, `LayersAndLevels`
     */
    enum class ImageExportMode : int16_t { LAYERS_AND_LEVELS, NONE, ONE_IMAGE_PER_LAYER, ONE_IMAGE_PER_LEVEL };

    /**
     * This file is a JSON schema of files created by LDtk level editor (https://ldtk.io).
     *
     * This is the root of any Project JSON file. It contains:  - the project settings, - an
     * array of levels, - a group of definitions (that can probably be safely ignored for most
     * users).
     */
    struct Ldtk {
        /**
         * This object is not actually used by LDtk. It ONLY exists to force explicit references to
         * all types, to make sure QuickType finds them and integrate all of them. Otherwise,
         * Quicktype will drop types that are not explicitely used.
         */
        std::optional<ForcedRefs> forced_refs;
        /**
         * LDtk application build identifier.<br/>  This is only used to identify the LDtk version
         * that generated this particular project file, which can be useful for specific bug fixing.
         * Note that the build identifier is just the date of the release, so it's not unique to
         * each user (one single global ID per LDtk public release), and as a result, completely
         * anonymous.
         */
        double app_build_id;
        /**
         * Number of backup files to keep, if the `backupOnSave` is TRUE
         */
        int64_t backup_limit;
        /**
         * If TRUE, an extra copy of the project will be created in a sub folder, when saving.
         */
        bool backup_on_save;
        /**
         * Target relative path to store backup files
         */
        std::optional<std::string> backup_rel_path;
        /**
         * Project background color
         */
        std::string bg_color;
        /**
         * An array of command lines that can be ran manually by the user
         */
        std::vector<LdtkCustomCommand> custom_commands;
        /**
         * Default height for new entities
         */
        int64_t default_entity_height;
        /**
         * Default width for new entities
         */
        int64_t default_entity_width;
        /**
         * Default grid size for new layers
         */
        int64_t default_grid_size;
        /**
         * Default background color of levels
         */
        std::string default_level_bg_color;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Default new level height
         */
        std::optional<int64_t> default_level_height;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Default new level width
         */
        std::optional<int64_t> default_level_width;
        /**
         * Default X pivot (0 to 1) for new entities
         */
        double default_pivot_x;
        /**
         * Default Y pivot (0 to 1) for new entities
         */
        double default_pivot_y;
        /**
         * A structure containing all the definitions of this project
         */
        Definitions defs;
        /**
         * If the project isn't in MultiWorlds mode, this is the IID of the internal "dummy" World.
         */
        std::string dummy_world_iid;
        /**
         * If TRUE, the exported PNGs will include the level background (color or image).
         */
        bool export_level_bg;
        /**
         * **WARNING**: this deprecated value is no longer exported since version 0.9.3  Replaced
         * by: `imageExportMode`
         */
        std::optional<bool> export_png;
        /**
         * If TRUE, a Tiled compatible file will also be generated along with the LDtk JSON file
         * (default is FALSE)
         */
        bool export_tiled;
        /**
         * If TRUE, one file will be saved for the project (incl. all its definitions) and one file
         * in a sub-folder for each level.
         */
        bool external_levels;
        /**
         * An array containing various advanced flags (ie. options or other states). Possible
         * values: `DiscardPreCsvIntGrid`, `ExportOldTableOfContentData`,
         * `ExportPreCsvIntGridFormat`, `IgnoreBackupSuggest`, `PrependIndexToLevelFileNames`,
         * `MultiWorlds`, `UseMultilinesType`
         */
        std::vector<Flag> flags;
        /**
         * Naming convention for Identifiers (first-letter uppercase, full uppercase etc.) Possible
         * values: `Capitalize`, `Uppercase`, `Lowercase`, `Free`
         */
        IdentifierStyle identifier_style;
        /**
         * Unique project identifier
         */
        std::string iid;
        /**
         * "Image export" option when saving project. Possible values: `None`, `OneImagePerLayer`,
         * `OneImagePerLevel`, `LayersAndLevels`
         */
        ImageExportMode image_export_mode;
        /**
         * File format version
         */
        std::string json_version;
        /**
         * The default naming convention for level identifiers.
         */
        std::string level_name_pattern;
        /**
         * All levels. The order of this array is only relevant in `LinearHorizontal` and
         * `linearVertical` world layouts (see `worldLayout` value).<br/>  Otherwise, you should
         * refer to the `worldX`,`worldY` coordinates of each Level.
         */
        std::vector<Level> levels;
        /**
         * If TRUE, the Json is partially minified (no indentation, nor line breaks, default is
         * FALSE)
         */
        bool minify_json;
        /**
         * Next Unique integer ID available
         */
        int64_t next_uid;
        /**
         * File naming pattern for exported PNGs
         */
        std::optional<std::string> png_file_pattern;
        /**
         * If TRUE, a very simplified will be generated on saving, for quicker & easier engine
         * integration.
         */
        bool simplified_export;
        /**
         * All instances of entities that have their `exportToToc` flag enabled are listed in this
         * array.
         */
        std::vector<LdtkTableOfContentEntry> toc;
        /**
         * This optional description is used by LDtk Samples to show up some informations and
         * instructions.
         */
        std::optional<std::string> tutorial_desc;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Height of the world grid in pixels.
         */
        std::optional<int64_t> world_grid_height;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  Width of the world grid in pixels.
         */
        std::optional<int64_t> world_grid_width;
        /**
         * **WARNING**: this field will move to the `worlds` array after the "multi-worlds" update.
         * It will then be `null`. You can enable the Multi-worlds advanced project option to enable
         * the change immediately.<br/><br/>  An enum that describes how levels are organized in
         * this project (ie. linearly or in a 2D space). Possible values: &lt;`null`&gt;, `Free`,
         * `GridVania`, `LinearHorizontal`, `LinearVertical`
         */
        std::optional<WorldLayout> world_layout;
        /**
         * This array will be empty, unless you enable the Multi-Worlds in the project advanced
         * settings.<br/><br/> - in current version, a LDtk project file can only contain a single
         * world with multiple levels in it. In this case, levels and world layout related settings
         * are stored in the root of the JSON.<br/> - with "Multi-worlds" enabled, there will be a
         * `worlds` array in root, each world containing levels and layout settings. Basically, it's
         * pretty much only about moving the `levels` array to the `worlds` array, along with world
         * layout related values (eg. `worldGridWidth` etc).<br/><br/>If you want to start
         * supporting this future update easily, please refer to this documentation:
         * https://github.com/deepnight/ldtk/issues/231
         */
        std::vector<World> worlds;
    };
}

namespace ldtk {
    void from_json(const json & j, LdtkCustomCommand & x);
    void to_json(json & j, const LdtkCustomCommand & x);

    void from_json(const json & j, FieldDefinition & x);
    void to_json(json & j, const FieldDefinition & x);

    void from_json(const json & j, TilesetRectangle & x);
    void to_json(json & j, const TilesetRectangle & x);

    void from_json(const json & j, EntityDefinition & x);
    void to_json(json & j, const EntityDefinition & x);

    void from_json(const json & j, EnumValueDefinition & x);
    void to_json(json & j, const EnumValueDefinition & x);

    void from_json(const json & j, EnumDefinition & x);
    void to_json(json & j, const EnumDefinition & x);

    void from_json(const json & j, AutoLayerRuleDefinition & x);
    void to_json(json & j, const AutoLayerRuleDefinition & x);

    void from_json(const json & j, AutoLayerRuleGroup & x);
    void to_json(json & j, const AutoLayerRuleGroup & x);

    void from_json(const json & j, IntGridValueDefinition & x);
    void to_json(json & j, const IntGridValueDefinition & x);

    void from_json(const json & j, IntGridValueGroupDefinition & x);
    void to_json(json & j, const IntGridValueGroupDefinition & x);

    void from_json(const json & j, LayerDefinition & x);
    void to_json(json & j, const LayerDefinition & x);

    void from_json(const json & j, TileCustomMetadata & x);
    void to_json(json & j, const TileCustomMetadata & x);

    void from_json(const json & j, EnumTagValue & x);
    void to_json(json & j, const EnumTagValue & x);

    void from_json(const json & j, TilesetDefinition & x);
    void to_json(json & j, const TilesetDefinition & x);

    void from_json(const json & j, Definitions & x);
    void to_json(json & j, const Definitions & x);

    void from_json(const json & j, FieldInstance & x);
    void to_json(json & j, const FieldInstance & x);

    void from_json(const json & j, EntityInstance & x);
    void to_json(json & j, const EntityInstance & x);

    void from_json(const json & j, ReferenceToAnEntityInstance & x);
    void to_json(json & j, const ReferenceToAnEntityInstance & x);

    void from_json(const json & j, GridPoint & x);
    void to_json(json & j, const GridPoint & x);

    void from_json(const json & j, IntGridValueInstance & x);
    void to_json(json & j, const IntGridValueInstance & x);

    void from_json(const json & j, TileInstance & x);
    void to_json(json & j, const TileInstance & x);

    void from_json(const json & j, LayerInstance & x);
    void to_json(json & j, const LayerInstance & x);

    void from_json(const json & j, LevelBackgroundPosition & x);
    void to_json(json & j, const LevelBackgroundPosition & x);

    void from_json(const json & j, NeighbourLevel & x);
    void to_json(json & j, const NeighbourLevel & x);

    void from_json(const json & j, Level & x);
    void to_json(json & j, const Level & x);

    void from_json(const json & j, LdtkTocInstanceData & x);
    void to_json(json & j, const LdtkTocInstanceData & x);

    void from_json(const json & j, LdtkTableOfContentEntry & x);
    void to_json(json & j, const LdtkTableOfContentEntry & x);

    void from_json(const json & j, World & x);
    void to_json(json & j, const World & x);

    void from_json(const json & j, ForcedRefs & x);
    void to_json(json & j, const ForcedRefs & x);

    void from_json(const json & j, Ldtk & x);
    void to_json(json & j, const Ldtk & x);

    void from_json(const json & j, When & x);
    void to_json(json & j, const When & x);

    void from_json(const json & j, AllowedRefs & x);
    void to_json(json & j, const AllowedRefs & x);

    void from_json(const json & j, EditorDisplayMode & x);
    void to_json(json & j, const EditorDisplayMode & x);

    void from_json(const json & j, EditorDisplayPos & x);
    void to_json(json & j, const EditorDisplayPos & x);

    void from_json(const json & j, EditorLinkStyle & x);
    void to_json(json & j, const EditorLinkStyle & x);

    void from_json(const json & j, TextLanguageMode & x);
    void to_json(json & j, const TextLanguageMode & x);

    void from_json(const json & j, LimitBehavior & x);
    void to_json(json & j, const LimitBehavior & x);

    void from_json(const json & j, LimitScope & x);
    void to_json(json & j, const LimitScope & x);

    void from_json(const json & j, RenderMode & x);
    void to_json(json & j, const RenderMode & x);

    void from_json(const json & j, TileRenderMode & x);
    void to_json(json & j, const TileRenderMode & x);

    void from_json(const json & j, Checker & x);
    void to_json(json & j, const Checker & x);

    void from_json(const json & j, TileMode & x);
    void to_json(json & j, const TileMode & x);

    void from_json(const json & j, Type & x);
    void to_json(json & j, const Type & x);

    void from_json(const json & j, EmbedAtlas & x);
    void to_json(json & j, const EmbedAtlas & x);

    void from_json(const json & j, Flag & x);
    void to_json(json & j, const Flag & x);

    void from_json(const json & j, BgPos & x);
    void to_json(json & j, const BgPos & x);

    void from_json(const json & j, WorldLayout & x);
    void to_json(json & j, const WorldLayout & x);

    void from_json(const json & j, IdentifierStyle & x);
    void to_json(json & j, const IdentifierStyle & x);

    void from_json(const json & j, ImageExportMode & x);
    void to_json(json & j, const ImageExportMode & x);

    inline void from_json(const json & j, LdtkCustomCommand& x) {
        x.command = j.at("command").get<std::string>();
        x.when = j.at("when").get<When>();
    }

    inline void to_json(json & j, const LdtkCustomCommand & x) {
        j = json::object();
        j["command"] = x.command;
        j["when"] = x.when;
    }

    inline void from_json(const json & j, FieldDefinition& x) {
        x.type = j.at("__type").get<std::string>();
        x.accept_file_types = get_stack_optional<std::vector<std::string>>(j, "acceptFileTypes");
        x.allowed_refs = j.at("allowedRefs").get<AllowedRefs>();
        x.allowed_refs_entity_uid = get_stack_optional<int64_t>(j, "allowedRefsEntityUid");
        x.allowed_ref_tags = j.at("allowedRefTags").get<std::vector<std::string>>();
        x.allow_out_of_level_ref = j.at("allowOutOfLevelRef").get<bool>();
        x.array_max_length = get_stack_optional<int64_t>(j, "arrayMaxLength");
        x.array_min_length = get_stack_optional<int64_t>(j, "arrayMinLength");
        x.auto_chain_ref = j.at("autoChainRef").get<bool>();
        x.can_be_null = j.at("canBeNull").get<bool>();
        x.default_override = get_untyped(j, "defaultOverride");
        x.doc = get_stack_optional<std::string>(j, "doc");
        x.editor_always_show = j.at("editorAlwaysShow").get<bool>();
        x.editor_cut_long_values = j.at("editorCutLongValues").get<bool>();
        x.editor_display_color = get_stack_optional<std::string>(j, "editorDisplayColor");
        x.editor_display_mode = j.at("editorDisplayMode").get<EditorDisplayMode>();
        x.editor_display_pos = j.at("editorDisplayPos").get<EditorDisplayPos>();
        x.editor_display_scale = j.at("editorDisplayScale").get<double>();
        x.editor_link_style = j.at("editorLinkStyle").get<EditorLinkStyle>();
        x.editor_show_in_world = j.at("editorShowInWorld").get<bool>();
        x.editor_text_prefix = get_stack_optional<std::string>(j, "editorTextPrefix");
        x.editor_text_suffix = get_stack_optional<std::string>(j, "editorTextSuffix");
        x.export_to_toc = j.at("exportToToc").get<bool>();
        x.identifier = j.at("identifier").get<std::string>();
        x.is_array = j.at("isArray").get<bool>();
        x.max = get_stack_optional<double>(j, "max");
        x.min = get_stack_optional<double>(j, "min");
        x.regex = get_stack_optional<std::string>(j, "regex");
        x.searchable = j.at("searchable").get<bool>();
        x.symmetrical_ref = j.at("symmetricalRef").get<bool>();
        x.text_language_mode = get_stack_optional<TextLanguageMode>(j, "textLanguageMode");
        x.tileset_uid = get_stack_optional<int64_t>(j, "tilesetUid");
        x.field_definition_type = j.at("type").get<std::string>();
        x.uid = j.at("uid").get<int64_t>();
        x.use_for_smart_color = j.at("useForSmartColor").get<bool>();
    }

    inline void to_json(json & j, const FieldDefinition & x) {
        j = json::object();
        j["__type"] = x.type;
        j["acceptFileTypes"] = x.accept_file_types;
        j["allowedRefs"] = x.allowed_refs;
        j["allowedRefsEntityUid"] = x.allowed_refs_entity_uid;
        j["allowedRefTags"] = x.allowed_ref_tags;
        j["allowOutOfLevelRef"] = x.allow_out_of_level_ref;
        j["arrayMaxLength"] = x.array_max_length;
        j["arrayMinLength"] = x.array_min_length;
        j["autoChainRef"] = x.auto_chain_ref;
        j["canBeNull"] = x.can_be_null;
        j["defaultOverride"] = x.default_override;
        j["doc"] = x.doc;
        j["editorAlwaysShow"] = x.editor_always_show;
        j["editorCutLongValues"] = x.editor_cut_long_values;
        j["editorDisplayColor"] = x.editor_display_color;
        j["editorDisplayMode"] = x.editor_display_mode;
        j["editorDisplayPos"] = x.editor_display_pos;
        j["editorDisplayScale"] = x.editor_display_scale;
        j["editorLinkStyle"] = x.editor_link_style;
        j["editorShowInWorld"] = x.editor_show_in_world;
        j["editorTextPrefix"] = x.editor_text_prefix;
        j["editorTextSuffix"] = x.editor_text_suffix;
        j["exportToToc"] = x.export_to_toc;
        j["identifier"] = x.identifier;
        j["isArray"] = x.is_array;
        j["max"] = x.max;
        j["min"] = x.min;
        j["regex"] = x.regex;
        j["searchable"] = x.searchable;
        j["symmetricalRef"] = x.symmetrical_ref;
        j["textLanguageMode"] = x.text_language_mode;
        j["tilesetUid"] = x.tileset_uid;
        j["type"] = x.field_definition_type;
        j["uid"] = x.uid;
        j["useForSmartColor"] = x.use_for_smart_color;
    }

    inline void from_json(const json & j, TilesetRectangle& x) {
        x.h = j.at("h").get<int64_t>();
        x.tileset_uid = j.at("tilesetUid").get<int64_t>();
        x.w = j.at("w").get<int64_t>();
        x.x = j.at("x").get<int64_t>();
        x.y = j.at("y").get<int64_t>();
    }

    inline void to_json(json & j, const TilesetRectangle & x) {
        j = json::object();
        j["h"] = x.h;
        j["tilesetUid"] = x.tileset_uid;
        j["w"] = x.w;
        j["x"] = x.x;
        j["y"] = x.y;
    }

    inline void from_json(const json & j, EntityDefinition& x) {
        x.allow_out_of_bounds = j.at("allowOutOfBounds").get<bool>();
        x.color = j.at("color").get<std::string>();
        x.doc = get_stack_optional<std::string>(j, "doc");
        x.export_to_toc = j.at("exportToToc").get<bool>();
        x.field_defs = j.at("fieldDefs").get<std::vector<FieldDefinition>>();
        x.fill_opacity = j.at("fillOpacity").get<double>();
        x.height = j.at("height").get<int64_t>();
        x.hollow = j.at("hollow").get<bool>();
        x.identifier = j.at("identifier").get<std::string>();
        x.keep_aspect_ratio = j.at("keepAspectRatio").get<bool>();
        x.limit_behavior = j.at("limitBehavior").get<LimitBehavior>();
        x.limit_scope = j.at("limitScope").get<LimitScope>();
        x.line_opacity = j.at("lineOpacity").get<double>();
        x.max_count = j.at("maxCount").get<int64_t>();
        x.max_height = get_stack_optional<int64_t>(j, "maxHeight");
        x.max_width = get_stack_optional<int64_t>(j, "maxWidth");
        x.min_height = get_stack_optional<int64_t>(j, "minHeight");
        x.min_width = get_stack_optional<int64_t>(j, "minWidth");
        x.nine_slice_borders = j.at("nineSliceBorders").get<std::vector<int64_t>>();
        x.pivot_x = j.at("pivotX").get<double>();
        x.pivot_y = j.at("pivotY").get<double>();
        x.render_mode = j.at("renderMode").get<RenderMode>();
        x.resizable_x = j.at("resizableX").get<bool>();
        x.resizable_y = j.at("resizableY").get<bool>();
        x.show_name = j.at("showName").get<bool>();
        x.tags = j.at("tags").get<std::vector<std::string>>();
        x.tile_id = get_stack_optional<int64_t>(j, "tileId");
        x.tile_opacity = j.at("tileOpacity").get<double>();
        x.tile_rect = get_stack_optional<TilesetRectangle>(j, "tileRect");
        x.tile_render_mode = j.at("tileRenderMode").get<TileRenderMode>();
        x.tileset_id = get_stack_optional<int64_t>(j, "tilesetId");
        x.uid = j.at("uid").get<int64_t>();
        x.ui_tile_rect = get_stack_optional<TilesetRectangle>(j, "uiTileRect");
        x.width = j.at("width").get<int64_t>();
    }

    inline void to_json(json & j, const EntityDefinition & x) {
        j = json::object();
        j["allowOutOfBounds"] = x.allow_out_of_bounds;
        j["color"] = x.color;
        j["doc"] = x.doc;
        j["exportToToc"] = x.export_to_toc;
        j["fieldDefs"] = x.field_defs;
        j["fillOpacity"] = x.fill_opacity;
        j["height"] = x.height;
        j["hollow"] = x.hollow;
        j["identifier"] = x.identifier;
        j["keepAspectRatio"] = x.keep_aspect_ratio;
        j["limitBehavior"] = x.limit_behavior;
        j["limitScope"] = x.limit_scope;
        j["lineOpacity"] = x.line_opacity;
        j["maxCount"] = x.max_count;
        j["maxHeight"] = x.max_height;
        j["maxWidth"] = x.max_width;
        j["minHeight"] = x.min_height;
        j["minWidth"] = x.min_width;
        j["nineSliceBorders"] = x.nine_slice_borders;
        j["pivotX"] = x.pivot_x;
        j["pivotY"] = x.pivot_y;
        j["renderMode"] = x.render_mode;
        j["resizableX"] = x.resizable_x;
        j["resizableY"] = x.resizable_y;
        j["showName"] = x.show_name;
        j["tags"] = x.tags;
        j["tileId"] = x.tile_id;
        j["tileOpacity"] = x.tile_opacity;
        j["tileRect"] = x.tile_rect;
        j["tileRenderMode"] = x.tile_render_mode;
        j["tilesetId"] = x.tileset_id;
        j["uid"] = x.uid;
        j["uiTileRect"] = x.ui_tile_rect;
        j["width"] = x.width;
    }

    inline void from_json(const json & j, EnumValueDefinition& x) {
        x.tile_src_rect = get_stack_optional<std::vector<int64_t>>(j, "__tileSrcRect");
        x.color = j.at("color").get<int64_t>();
        x.id = j.at("id").get<std::string>();
        x.tile_id = get_stack_optional<int64_t>(j, "tileId");
        x.tile_rect = get_stack_optional<TilesetRectangle>(j, "tileRect");
    }

    inline void to_json(json & j, const EnumValueDefinition & x) {
        j = json::object();
        j["__tileSrcRect"] = x.tile_src_rect;
        j["color"] = x.color;
        j["id"] = x.id;
        j["tileId"] = x.tile_id;
        j["tileRect"] = x.tile_rect;
    }

    inline void from_json(const json & j, EnumDefinition& x) {
        x.external_file_checksum = get_stack_optional<std::string>(j, "externalFileChecksum");
        x.external_rel_path = get_stack_optional<std::string>(j, "externalRelPath");
        x.icon_tileset_uid = get_stack_optional<int64_t>(j, "iconTilesetUid");
        x.identifier = j.at("identifier").get<std::string>();
        x.tags = j.at("tags").get<std::vector<std::string>>();
        x.uid = j.at("uid").get<int64_t>();
        x.values = j.at("values").get<std::vector<EnumValueDefinition>>();
    }

    inline void to_json(json & j, const EnumDefinition & x) {
        j = json::object();
        j["externalFileChecksum"] = x.external_file_checksum;
        j["externalRelPath"] = x.external_rel_path;
        j["iconTilesetUid"] = x.icon_tileset_uid;
        j["identifier"] = x.identifier;
        j["tags"] = x.tags;
        j["uid"] = x.uid;
        j["values"] = x.values;
    }

    inline void from_json(const json & j, AutoLayerRuleDefinition& x) {
        x.active = j.at("active").get<bool>();
        x.alpha = j.at("alpha").get<double>();
        x.break_on_match = j.at("breakOnMatch").get<bool>();
        x.chance = j.at("chance").get<double>();
        x.checker = j.at("checker").get<Checker>();
        x.flip_x = j.at("flipX").get<bool>();
        x.flip_y = j.at("flipY").get<bool>();
        x.invalidated = j.at("invalidated").get<bool>();
        x.out_of_bounds_value = get_stack_optional<int64_t>(j, "outOfBoundsValue");
        x.pattern = j.at("pattern").get<std::vector<int64_t>>();
        x.perlin_active = j.at("perlinActive").get<bool>();
        x.perlin_octaves = j.at("perlinOctaves").get<double>();
        x.perlin_scale = j.at("perlinScale").get<double>();
        x.perlin_seed = j.at("perlinSeed").get<double>();
        x.pivot_x = j.at("pivotX").get<double>();
        x.pivot_y = j.at("pivotY").get<double>();
        x.size = j.at("size").get<int64_t>();
        x.tile_ids = get_stack_optional<std::vector<int64_t>>(j, "tileIds");
        x.tile_mode = j.at("tileMode").get<TileMode>();
        x.tile_random_x_max = j.at("tileRandomXMax").get<int64_t>();
        x.tile_random_x_min = j.at("tileRandomXMin").get<int64_t>();
        x.tile_random_y_max = j.at("tileRandomYMax").get<int64_t>();
        x.tile_random_y_min = j.at("tileRandomYMin").get<int64_t>();
        x.tile_rects_ids = j.at("tileRectsIds").get<std::vector<std::vector<int64_t>>>();
        x.tile_x_offset = j.at("tileXOffset").get<int64_t>();
        x.tile_y_offset = j.at("tileYOffset").get<int64_t>();
        x.uid = j.at("uid").get<int64_t>();
        x.x_modulo = j.at("xModulo").get<int64_t>();
        x.x_offset = j.at("xOffset").get<int64_t>();
        x.y_modulo = j.at("yModulo").get<int64_t>();
        x.y_offset = j.at("yOffset").get<int64_t>();
    }

    inline void to_json(json & j, const AutoLayerRuleDefinition & x) {
        j = json::object();
        j["active"] = x.active;
        j["alpha"] = x.alpha;
        j["breakOnMatch"] = x.break_on_match;
        j["chance"] = x.chance;
        j["checker"] = x.checker;
        j["flipX"] = x.flip_x;
        j["flipY"] = x.flip_y;
        j["invalidated"] = x.invalidated;
        j["outOfBoundsValue"] = x.out_of_bounds_value;
        j["pattern"] = x.pattern;
        j["perlinActive"] = x.perlin_active;
        j["perlinOctaves"] = x.perlin_octaves;
        j["perlinScale"] = x.perlin_scale;
        j["perlinSeed"] = x.perlin_seed;
        j["pivotX"] = x.pivot_x;
        j["pivotY"] = x.pivot_y;
        j["size"] = x.size;
        j["tileIds"] = x.tile_ids;
        j["tileMode"] = x.tile_mode;
        j["tileRandomXMax"] = x.tile_random_x_max;
        j["tileRandomXMin"] = x.tile_random_x_min;
        j["tileRandomYMax"] = x.tile_random_y_max;
        j["tileRandomYMin"] = x.tile_random_y_min;
        j["tileRectsIds"] = x.tile_rects_ids;
        j["tileXOffset"] = x.tile_x_offset;
        j["tileYOffset"] = x.tile_y_offset;
        j["uid"] = x.uid;
        j["xModulo"] = x.x_modulo;
        j["xOffset"] = x.x_offset;
        j["yModulo"] = x.y_modulo;
        j["yOffset"] = x.y_offset;
    }

    inline void from_json(const json & j, AutoLayerRuleGroup& x) {
        x.active = j.at("active").get<bool>();
        x.biome_requirement_mode = j.at("biomeRequirementMode").get<int64_t>();
        x.collapsed = get_stack_optional<bool>(j, "collapsed");
        x.color = get_stack_optional<std::string>(j, "color");
        x.icon = get_stack_optional<TilesetRectangle>(j, "icon");
        x.is_optional = j.at("isOptional").get<bool>();
        x.name = j.at("name").get<std::string>();
        x.required_biome_values = j.at("requiredBiomeValues").get<std::vector<std::string>>();
        x.rules = j.at("rules").get<std::vector<AutoLayerRuleDefinition>>();
        x.uid = j.at("uid").get<int64_t>();
        x.uses_wizard = j.at("usesWizard").get<bool>();
    }

    inline void to_json(json & j, const AutoLayerRuleGroup & x) {
        j = json::object();
        j["active"] = x.active;
        j["biomeRequirementMode"] = x.biome_requirement_mode;
        j["collapsed"] = x.collapsed;
        j["color"] = x.color;
        j["icon"] = x.icon;
        j["isOptional"] = x.is_optional;
        j["name"] = x.name;
        j["requiredBiomeValues"] = x.required_biome_values;
        j["rules"] = x.rules;
        j["uid"] = x.uid;
        j["usesWizard"] = x.uses_wizard;
    }

    inline void from_json(const json & j, IntGridValueDefinition& x) {
        x.color = j.at("color").get<std::string>();
        x.group_uid = j.at("groupUid").get<int64_t>();
        x.identifier = get_stack_optional<std::string>(j, "identifier");
        x.tile = get_stack_optional<TilesetRectangle>(j, "tile");
        x.value = j.at("value").get<int64_t>();
    }

    inline void to_json(json & j, const IntGridValueDefinition & x) {
        j = json::object();
        j["color"] = x.color;
        j["groupUid"] = x.group_uid;
        j["identifier"] = x.identifier;
        j["tile"] = x.tile;
        j["value"] = x.value;
    }

    inline void from_json(const json & j, IntGridValueGroupDefinition& x) {
        x.color = get_stack_optional<std::string>(j, "color");
        x.identifier = get_stack_optional<std::string>(j, "identifier");
        x.uid = j.at("uid").get<int64_t>();
    }

    inline void to_json(json & j, const IntGridValueGroupDefinition & x) {
        j = json::object();
        j["color"] = x.color;
        j["identifier"] = x.identifier;
        j["uid"] = x.uid;
    }

    inline void from_json(const json & j, LayerDefinition& x) {
        x.type = j.at("__type").get<std::string>();
        x.auto_rule_groups = j.at("autoRuleGroups").get<std::vector<AutoLayerRuleGroup>>();
        x.auto_source_layer_def_uid = get_stack_optional<int64_t>(j, "autoSourceLayerDefUid");
        x.auto_tileset_def_uid = get_stack_optional<int64_t>(j, "autoTilesetDefUid");
        x.auto_tiles_killed_by_other_layer_uid = get_stack_optional<int64_t>(j, "autoTilesKilledByOtherLayerUid");
        x.biome_field_uid = get_stack_optional<int64_t>(j, "biomeFieldUid");
        x.can_select_when_inactive = j.at("canSelectWhenInactive").get<bool>();
        x.display_opacity = j.at("displayOpacity").get<double>();
        x.doc = get_stack_optional<std::string>(j, "doc");
        x.excluded_tags = j.at("excludedTags").get<std::vector<std::string>>();
        x.grid_size = j.at("gridSize").get<int64_t>();
        x.guide_grid_hei = j.at("guideGridHei").get<int64_t>();
        x.guide_grid_wid = j.at("guideGridWid").get<int64_t>();
        x.hide_fields_when_inactive = j.at("hideFieldsWhenInactive").get<bool>();
        x.hide_in_list = j.at("hideInList").get<bool>();
        x.identifier = j.at("identifier").get<std::string>();
        x.inactive_opacity = j.at("inactiveOpacity").get<double>();
        x.int_grid_values = j.at("intGridValues").get<std::vector<IntGridValueDefinition>>();
        x.int_grid_values_groups = j.at("intGridValuesGroups").get<std::vector<IntGridValueGroupDefinition>>();
        x.parallax_factor_x = j.at("parallaxFactorX").get<double>();
        x.parallax_factor_y = j.at("parallaxFactorY").get<double>();
        x.parallax_scaling = j.at("parallaxScaling").get<bool>();
        x.px_offset_x = j.at("pxOffsetX").get<int64_t>();
        x.px_offset_y = j.at("pxOffsetY").get<int64_t>();
        x.render_in_world_view = j.at("renderInWorldView").get<bool>();
        x.required_tags = j.at("requiredTags").get<std::vector<std::string>>();
        x.tile_pivot_x = j.at("tilePivotX").get<double>();
        x.tile_pivot_y = j.at("tilePivotY").get<double>();
        x.tileset_def_uid = get_stack_optional<int64_t>(j, "tilesetDefUid");
        x.layer_definition_type = j.at("type").get<Type>();
        x.ui_color = get_stack_optional<std::string>(j, "uiColor");
        x.uid = j.at("uid").get<int64_t>();
        x.ui_filter_tags = j.at("uiFilterTags").get<std::vector<std::string>>();
        x.use_async_render = j.at("useAsyncRender").get<bool>();
    }

    inline void to_json(json & j, const LayerDefinition & x) {
        j = json::object();
        j["__type"] = x.type;
        j["autoRuleGroups"] = x.auto_rule_groups;
        j["autoSourceLayerDefUid"] = x.auto_source_layer_def_uid;
        j["autoTilesetDefUid"] = x.auto_tileset_def_uid;
        j["autoTilesKilledByOtherLayerUid"] = x.auto_tiles_killed_by_other_layer_uid;
        j["biomeFieldUid"] = x.biome_field_uid;
        j["canSelectWhenInactive"] = x.can_select_when_inactive;
        j["displayOpacity"] = x.display_opacity;
        j["doc"] = x.doc;
        j["excludedTags"] = x.excluded_tags;
        j["gridSize"] = x.grid_size;
        j["guideGridHei"] = x.guide_grid_hei;
        j["guideGridWid"] = x.guide_grid_wid;
        j["hideFieldsWhenInactive"] = x.hide_fields_when_inactive;
        j["hideInList"] = x.hide_in_list;
        j["identifier"] = x.identifier;
        j["inactiveOpacity"] = x.inactive_opacity;
        j["intGridValues"] = x.int_grid_values;
        j["intGridValuesGroups"] = x.int_grid_values_groups;
        j["parallaxFactorX"] = x.parallax_factor_x;
        j["parallaxFactorY"] = x.parallax_factor_y;
        j["parallaxScaling"] = x.parallax_scaling;
        j["pxOffsetX"] = x.px_offset_x;
        j["pxOffsetY"] = x.px_offset_y;
        j["renderInWorldView"] = x.render_in_world_view;
        j["requiredTags"] = x.required_tags;
        j["tilePivotX"] = x.tile_pivot_x;
        j["tilePivotY"] = x.tile_pivot_y;
        j["tilesetDefUid"] = x.tileset_def_uid;
        j["type"] = x.layer_definition_type;
        j["uiColor"] = x.ui_color;
        j["uid"] = x.uid;
        j["uiFilterTags"] = x.ui_filter_tags;
        j["useAsyncRender"] = x.use_async_render;
    }

    inline void from_json(const json & j, TileCustomMetadata& x) {
        x.data = j.at("data").get<std::string>();
        x.tile_id = j.at("tileId").get<int64_t>();
    }

    inline void to_json(json & j, const TileCustomMetadata & x) {
        j = json::object();
        j["data"] = x.data;
        j["tileId"] = x.tile_id;
    }

    inline void from_json(const json & j, EnumTagValue& x) {
        x.enum_value_id = j.at("enumValueId").get<std::string>();
        x.tile_ids = j.at("tileIds").get<std::vector<int64_t>>();
    }

    inline void to_json(json & j, const EnumTagValue & x) {
        j = json::object();
        j["enumValueId"] = x.enum_value_id;
        j["tileIds"] = x.tile_ids;
    }

    inline void from_json(const json & j, TilesetDefinition& x) {
        x.c_hei = j.at("__cHei").get<int64_t>();
        x.c_wid = j.at("__cWid").get<int64_t>();
        x.cached_pixel_data = get_stack_optional<std::map<std::string, nlohmann::json>>(j, "cachedPixelData");
        x.custom_data = j.at("customData").get<std::vector<TileCustomMetadata>>();
        x.embed_atlas = get_stack_optional<EmbedAtlas>(j, "embedAtlas");
        x.enum_tags = j.at("enumTags").get<std::vector<EnumTagValue>>();
        x.identifier = j.at("identifier").get<std::string>();
        x.padding = j.at("padding").get<int64_t>();
        x.px_hei = j.at("pxHei").get<int64_t>();
        x.px_wid = j.at("pxWid").get<int64_t>();
        x.rel_path = get_stack_optional<std::string>(j, "relPath");
        x.saved_selections = j.at("savedSelections").get<std::vector<std::map<std::string, nlohmann::json>>>();
        x.spacing = j.at("spacing").get<int64_t>();
        x.tags = j.at("tags").get<std::vector<std::string>>();
        x.tags_source_enum_uid = get_stack_optional<int64_t>(j, "tagsSourceEnumUid");
        x.tile_grid_size = j.at("tileGridSize").get<int64_t>();
        x.uid = j.at("uid").get<int64_t>();
    }

    inline void to_json(json & j, const TilesetDefinition & x) {
        j = json::object();
        j["__cHei"] = x.c_hei;
        j["__cWid"] = x.c_wid;
        j["cachedPixelData"] = x.cached_pixel_data;
        j["customData"] = x.custom_data;
        j["embedAtlas"] = x.embed_atlas;
        j["enumTags"] = x.enum_tags;
        j["identifier"] = x.identifier;
        j["padding"] = x.padding;
        j["pxHei"] = x.px_hei;
        j["pxWid"] = x.px_wid;
        j["relPath"] = x.rel_path;
        j["savedSelections"] = x.saved_selections;
        j["spacing"] = x.spacing;
        j["tags"] = x.tags;
        j["tagsSourceEnumUid"] = x.tags_source_enum_uid;
        j["tileGridSize"] = x.tile_grid_size;
        j["uid"] = x.uid;
    }

    inline void from_json(const json & j, Definitions& x) {
        x.entities = j.at("entities").get<std::vector<EntityDefinition>>();
        x.enums = j.at("enums").get<std::vector<EnumDefinition>>();
        x.external_enums = j.at("externalEnums").get<std::vector<EnumDefinition>>();
        x.layers = j.at("layers").get<std::vector<LayerDefinition>>();
        x.level_fields = j.at("levelFields").get<std::vector<FieldDefinition>>();
        x.tilesets = j.at("tilesets").get<std::vector<TilesetDefinition>>();
    }

    inline void to_json(json & j, const Definitions & x) {
        j = json::object();
        j["entities"] = x.entities;
        j["enums"] = x.enums;
        j["externalEnums"] = x.external_enums;
        j["layers"] = x.layers;
        j["levelFields"] = x.level_fields;
        j["tilesets"] = x.tilesets;
    }

    inline void from_json(const json & j, FieldInstance& x) {
        x.identifier = j.at("__identifier").get<std::string>();
        x.tile = get_stack_optional<TilesetRectangle>(j, "__tile");
        x.type = j.at("__type").get<std::string>();
        x.value = get_untyped(j, "__value");
        x.def_uid = j.at("defUid").get<int64_t>();
        x.real_editor_values = j.at("realEditorValues").get<std::vector<nlohmann::json>>();
    }

    inline void to_json(json & j, const FieldInstance & x) {
        j = json::object();
        j["__identifier"] = x.identifier;
        j["__tile"] = x.tile;
        j["__type"] = x.type;
        j["__value"] = x.value;
        j["defUid"] = x.def_uid;
        j["realEditorValues"] = x.real_editor_values;
    }

    inline void from_json(const json & j, EntityInstance& x) {
        x.grid = j.at("__grid").get<std::vector<int64_t>>();
        x.identifier = j.at("__identifier").get<std::string>();
        x.pivot = j.at("__pivot").get<std::vector<double>>();
        x.smart_color = j.at("__smartColor").get<std::string>();
        x.tags = j.at("__tags").get<std::vector<std::string>>();
        x.tile = get_stack_optional<TilesetRectangle>(j, "__tile");
        x.world_x = get_stack_optional<int64_t>(j, "__worldX");
        x.world_y = get_stack_optional<int64_t>(j, "__worldY");
        x.def_uid = j.at("defUid").get<int64_t>();
        x.field_instances = j.at("fieldInstances").get<std::vector<FieldInstance>>();
        x.height = j.at("height").get<int64_t>();
        x.iid = j.at("iid").get<std::string>();
        x.px = j.at("px").get<std::vector<int64_t>>();
        x.width = j.at("width").get<int64_t>();
    }

    inline void to_json(json & j, const EntityInstance & x) {
        j = json::object();
        j["__grid"] = x.grid;
        j["__identifier"] = x.identifier;
        j["__pivot"] = x.pivot;
        j["__smartColor"] = x.smart_color;
        j["__tags"] = x.tags;
        j["__tile"] = x.tile;
        j["__worldX"] = x.world_x;
        j["__worldY"] = x.world_y;
        j["defUid"] = x.def_uid;
        j["fieldInstances"] = x.field_instances;
        j["height"] = x.height;
        j["iid"] = x.iid;
        j["px"] = x.px;
        j["width"] = x.width;
    }

    inline void from_json(const json & j, ReferenceToAnEntityInstance& x) {
        x.entity_iid = j.at("entityIid").get<std::string>();
        x.layer_iid = j.at("layerIid").get<std::string>();
        x.level_iid = j.at("levelIid").get<std::string>();
        x.world_iid = j.at("worldIid").get<std::string>();
    }

    inline void to_json(json & j, const ReferenceToAnEntityInstance & x) {
        j = json::object();
        j["entityIid"] = x.entity_iid;
        j["layerIid"] = x.layer_iid;
        j["levelIid"] = x.level_iid;
        j["worldIid"] = x.world_iid;
    }

    inline void from_json(const json & j, GridPoint& x) {
        x.cx = j.at("cx").get<int64_t>();
        x.cy = j.at("cy").get<int64_t>();
    }

    inline void to_json(json & j, const GridPoint & x) {
        j = json::object();
        j["cx"] = x.cx;
        j["cy"] = x.cy;
    }

    inline void from_json(const json & j, IntGridValueInstance& x) {
        x.coord_id = j.at("coordId").get<int64_t>();
        x.v = j.at("v").get<int64_t>();
    }

    inline void to_json(json & j, const IntGridValueInstance & x) {
        j = json::object();
        j["coordId"] = x.coord_id;
        j["v"] = x.v;
    }

    inline void from_json(const json & j, TileInstance& x) {
        x.a = j.at("a").get<double>();
        x.d = j.at("d").get<std::vector<int64_t>>();
        x.f = j.at("f").get<int64_t>();
        x.px = j.at("px").get<std::vector<int64_t>>();
        x.src = j.at("src").get<std::vector<int64_t>>();
        x.t = j.at("t").get<int64_t>();
    }

    inline void to_json(json & j, const TileInstance & x) {
        j = json::object();
        j["a"] = x.a;
        j["d"] = x.d;
        j["f"] = x.f;
        j["px"] = x.px;
        j["src"] = x.src;
        j["t"] = x.t;
    }

    inline void from_json(const json & j, LayerInstance& x) {
        x.c_hei = j.at("__cHei").get<int64_t>();
        x.c_wid = j.at("__cWid").get<int64_t>();
        x.grid_size = j.at("__gridSize").get<int64_t>();
        x.identifier = j.at("__identifier").get<std::string>();
        x.opacity = j.at("__opacity").get<double>();
        x.px_total_offset_x = j.at("__pxTotalOffsetX").get<int64_t>();
        x.px_total_offset_y = j.at("__pxTotalOffsetY").get<int64_t>();
        x.tileset_def_uid = get_stack_optional<int64_t>(j, "__tilesetDefUid");
        x.tileset_rel_path = get_stack_optional<std::string>(j, "__tilesetRelPath");
        x.type = j.at("__type").get<std::string>();
        x.auto_layer_tiles = j.at("autoLayerTiles").get<std::vector<TileInstance>>();
        x.entity_instances = j.at("entityInstances").get<std::vector<EntityInstance>>();
        x.grid_tiles = j.at("gridTiles").get<std::vector<TileInstance>>();
        x.iid = j.at("iid").get<std::string>();
        x.int_grid = get_stack_optional<std::vector<IntGridValueInstance>>(j, "intGrid");
        x.int_grid_csv = j.at("intGridCsv").get<std::vector<int64_t>>();
        x.layer_def_uid = j.at("layerDefUid").get<int64_t>();
        x.level_id = j.at("levelId").get<int64_t>();
        x.optional_rules = j.at("optionalRules").get<std::vector<int64_t>>();
        x.override_tileset_uid = get_stack_optional<int64_t>(j, "overrideTilesetUid");
        x.px_offset_x = j.at("pxOffsetX").get<int64_t>();
        x.px_offset_y = j.at("pxOffsetY").get<int64_t>();
        x.seed = j.at("seed").get<int64_t>();
        x.visible = j.at("visible").get<bool>();
    }

    inline void to_json(json & j, const LayerInstance & x) {
        j = json::object();
        j["__cHei"] = x.c_hei;
        j["__cWid"] = x.c_wid;
        j["__gridSize"] = x.grid_size;
        j["__identifier"] = x.identifier;
        j["__opacity"] = x.opacity;
        j["__pxTotalOffsetX"] = x.px_total_offset_x;
        j["__pxTotalOffsetY"] = x.px_total_offset_y;
        j["__tilesetDefUid"] = x.tileset_def_uid;
        j["__tilesetRelPath"] = x.tileset_rel_path;
        j["__type"] = x.type;
        j["autoLayerTiles"] = x.auto_layer_tiles;
        j["entityInstances"] = x.entity_instances;
        j["gridTiles"] = x.grid_tiles;
        j["iid"] = x.iid;
        j["intGrid"] = x.int_grid;
        j["intGridCsv"] = x.int_grid_csv;
        j["layerDefUid"] = x.layer_def_uid;
        j["levelId"] = x.level_id;
        j["optionalRules"] = x.optional_rules;
        j["overrideTilesetUid"] = x.override_tileset_uid;
        j["pxOffsetX"] = x.px_offset_x;
        j["pxOffsetY"] = x.px_offset_y;
        j["seed"] = x.seed;
        j["visible"] = x.visible;
    }

    inline void from_json(const json & j, LevelBackgroundPosition& x) {
        x.crop_rect = j.at("cropRect").get<std::vector<double>>();
        x.scale = j.at("scale").get<std::vector<double>>();
        x.top_left_px = j.at("topLeftPx").get<std::vector<int64_t>>();
    }

    inline void to_json(json & j, const LevelBackgroundPosition & x) {
        j = json::object();
        j["cropRect"] = x.crop_rect;
        j["scale"] = x.scale;
        j["topLeftPx"] = x.top_left_px;
    }

    inline void from_json(const json & j, NeighbourLevel& x) {
        x.dir = j.at("dir").get<std::string>();
        x.level_iid = j.at("levelIid").get<std::string>();
        x.level_uid = get_stack_optional<int64_t>(j, "levelUid");
    }

    inline void to_json(json & j, const NeighbourLevel & x) {
        j = json::object();
        j["dir"] = x.dir;
        j["levelIid"] = x.level_iid;
        j["levelUid"] = x.level_uid;
    }

    inline void from_json(const json & j, Level& x) {
        x.bg_color = j.at("__bgColor").get<std::string>();
        x.bg_pos = get_stack_optional<LevelBackgroundPosition>(j, "__bgPos");
        x.neighbours = j.at("__neighbours").get<std::vector<NeighbourLevel>>();
        x.smart_color = j.at("__smartColor").get<std::string>();
        x.level_bg_color = get_stack_optional<std::string>(j, "bgColor");
        x.bg_pivot_x = j.at("bgPivotX").get<double>();
        x.bg_pivot_y = j.at("bgPivotY").get<double>();
        x.level_bg_pos = get_stack_optional<BgPos>(j, "bgPos");
        x.bg_rel_path = get_stack_optional<std::string>(j, "bgRelPath");
        x.external_rel_path = get_stack_optional<std::string>(j, "externalRelPath");
        x.field_instances = j.at("fieldInstances").get<std::vector<FieldInstance>>();
        x.identifier = j.at("identifier").get<std::string>();
        x.iid = j.at("iid").get<std::string>();
        x.layer_instances = get_stack_optional<std::vector<LayerInstance>>(j, "layerInstances");
        x.px_hei = j.at("pxHei").get<int64_t>();
        x.px_wid = j.at("pxWid").get<int64_t>();
        x.uid = j.at("uid").get<int64_t>();
        x.use_auto_identifier = j.at("useAutoIdentifier").get<bool>();
        x.world_depth = j.at("worldDepth").get<int64_t>();
        x.world_x = j.at("worldX").get<int64_t>();
        x.world_y = j.at("worldY").get<int64_t>();
    }

    inline void to_json(json & j, const Level & x) {
        j = json::object();
        j["__bgColor"] = x.bg_color;
        j["__bgPos"] = x.bg_pos;
        j["__neighbours"] = x.neighbours;
        j["__smartColor"] = x.smart_color;
        j["bgColor"] = x.level_bg_color;
        j["bgPivotX"] = x.bg_pivot_x;
        j["bgPivotY"] = x.bg_pivot_y;
        j["bgPos"] = x.level_bg_pos;
        j["bgRelPath"] = x.bg_rel_path;
        j["externalRelPath"] = x.external_rel_path;
        j["fieldInstances"] = x.field_instances;
        j["identifier"] = x.identifier;
        j["iid"] = x.iid;
        j["layerInstances"] = x.layer_instances;
        j["pxHei"] = x.px_hei;
        j["pxWid"] = x.px_wid;
        j["uid"] = x.uid;
        j["useAutoIdentifier"] = x.use_auto_identifier;
        j["worldDepth"] = x.world_depth;
        j["worldX"] = x.world_x;
        j["worldY"] = x.world_y;
    }

    inline void from_json(const json & j, LdtkTocInstanceData& x) {
        x.fields = get_untyped(j, "fields");
        x.hei_px = j.at("heiPx").get<int64_t>();
        x.iids = j.at("iids").get<ReferenceToAnEntityInstance>();
        x.wid_px = j.at("widPx").get<int64_t>();
        x.world_x = j.at("worldX").get<int64_t>();
        x.world_y = j.at("worldY").get<int64_t>();
    }

    inline void to_json(json & j, const LdtkTocInstanceData & x) {
        j = json::object();
        j["fields"] = x.fields;
        j["heiPx"] = x.hei_px;
        j["iids"] = x.iids;
        j["widPx"] = x.wid_px;
        j["worldX"] = x.world_x;
        j["worldY"] = x.world_y;
    }

    inline void from_json(const json & j, LdtkTableOfContentEntry& x) {
        x.identifier = j.at("identifier").get<std::string>();
        x.instances = get_stack_optional<std::vector<ReferenceToAnEntityInstance>>(j, "instances");
        x.instances_data = j.at("instancesData").get<std::vector<LdtkTocInstanceData>>();
    }

    inline void to_json(json & j, const LdtkTableOfContentEntry & x) {
        j = json::object();
        j["identifier"] = x.identifier;
        j["instances"] = x.instances;
        j["instancesData"] = x.instances_data;
    }

    inline void from_json(const json & j, World& x) {
        x.default_level_height = j.at("defaultLevelHeight").get<int64_t>();
        x.default_level_width = j.at("defaultLevelWidth").get<int64_t>();
        x.identifier = j.at("identifier").get<std::string>();
        x.iid = j.at("iid").get<std::string>();
        x.levels = j.at("levels").get<std::vector<Level>>();
        x.world_grid_height = j.at("worldGridHeight").get<int64_t>();
        x.world_grid_width = j.at("worldGridWidth").get<int64_t>();
        x.world_layout = get_stack_optional<WorldLayout>(j, "worldLayout");
    }

    inline void to_json(json & j, const World & x) {
        j = json::object();
        j["defaultLevelHeight"] = x.default_level_height;
        j["defaultLevelWidth"] = x.default_level_width;
        j["identifier"] = x.identifier;
        j["iid"] = x.iid;
        j["levels"] = x.levels;
        j["worldGridHeight"] = x.world_grid_height;
        j["worldGridWidth"] = x.world_grid_width;
        j["worldLayout"] = x.world_layout;
    }

    inline void from_json(const json & j, ForcedRefs& x) {
        x.auto_layer_rule_group = get_stack_optional<AutoLayerRuleGroup>(j, "AutoLayerRuleGroup");
        x.auto_rule_def = get_stack_optional<AutoLayerRuleDefinition>(j, "AutoRuleDef");
        x.custom_command = get_stack_optional<LdtkCustomCommand>(j, "CustomCommand");
        x.definitions = get_stack_optional<Definitions>(j, "Definitions");
        x.entity_def = get_stack_optional<EntityDefinition>(j, "EntityDef");
        x.entity_instance = get_stack_optional<EntityInstance>(j, "EntityInstance");
        x.entity_reference_infos = get_stack_optional<ReferenceToAnEntityInstance>(j, "EntityReferenceInfos");
        x.enum_def = get_stack_optional<EnumDefinition>(j, "EnumDef");
        x.enum_def_values = get_stack_optional<EnumValueDefinition>(j, "EnumDefValues");
        x.enum_tag_value = get_stack_optional<EnumTagValue>(j, "EnumTagValue");
        x.field_def = get_stack_optional<FieldDefinition>(j, "FieldDef");
        x.field_instance = get_stack_optional<FieldInstance>(j, "FieldInstance");
        x.grid_point = get_stack_optional<GridPoint>(j, "GridPoint");
        x.int_grid_value_def = get_stack_optional<IntGridValueDefinition>(j, "IntGridValueDef");
        x.int_grid_value_group_def = get_stack_optional<IntGridValueGroupDefinition>(j, "IntGridValueGroupDef");
        x.int_grid_value_instance = get_stack_optional<IntGridValueInstance>(j, "IntGridValueInstance");
        x.layer_def = get_stack_optional<LayerDefinition>(j, "LayerDef");
        x.layer_instance = get_stack_optional<LayerInstance>(j, "LayerInstance");
        x.level = get_stack_optional<Level>(j, "Level");
        x.level_bg_pos_infos = get_stack_optional<LevelBackgroundPosition>(j, "LevelBgPosInfos");
        x.neighbour_level = get_stack_optional<NeighbourLevel>(j, "NeighbourLevel");
        x.table_of_content_entry = get_stack_optional<LdtkTableOfContentEntry>(j, "TableOfContentEntry");
        x.tile = get_stack_optional<TileInstance>(j, "Tile");
        x.tile_custom_metadata = get_stack_optional<TileCustomMetadata>(j, "TileCustomMetadata");
        x.tileset_def = get_stack_optional<TilesetDefinition>(j, "TilesetDef");
        x.tileset_rect = get_stack_optional<TilesetRectangle>(j, "TilesetRect");
        x.toc_instance_data = get_stack_optional<LdtkTocInstanceData>(j, "TocInstanceData");
        x.world = get_stack_optional<World>(j, "World");
    }

    inline void to_json(json & j, const ForcedRefs & x) {
        j = json::object();
        j["AutoLayerRuleGroup"] = x.auto_layer_rule_group;
        j["AutoRuleDef"] = x.auto_rule_def;
        j["CustomCommand"] = x.custom_command;
        j["Definitions"] = x.definitions;
        j["EntityDef"] = x.entity_def;
        j["EntityInstance"] = x.entity_instance;
        j["EntityReferenceInfos"] = x.entity_reference_infos;
        j["EnumDef"] = x.enum_def;
        j["EnumDefValues"] = x.enum_def_values;
        j["EnumTagValue"] = x.enum_tag_value;
        j["FieldDef"] = x.field_def;
        j["FieldInstance"] = x.field_instance;
        j["GridPoint"] = x.grid_point;
        j["IntGridValueDef"] = x.int_grid_value_def;
        j["IntGridValueGroupDef"] = x.int_grid_value_group_def;
        j["IntGridValueInstance"] = x.int_grid_value_instance;
        j["LayerDef"] = x.layer_def;
        j["LayerInstance"] = x.layer_instance;
        j["Level"] = x.level;
        j["LevelBgPosInfos"] = x.level_bg_pos_infos;
        j["NeighbourLevel"] = x.neighbour_level;
        j["TableOfContentEntry"] = x.table_of_content_entry;
        j["Tile"] = x.tile;
        j["TileCustomMetadata"] = x.tile_custom_metadata;
        j["TilesetDef"] = x.tileset_def;
        j["TilesetRect"] = x.tileset_rect;
        j["TocInstanceData"] = x.toc_instance_data;
        j["World"] = x.world;
    }

    inline void from_json(const json & j, Ldtk& x) {
        x.forced_refs = get_stack_optional<ForcedRefs>(j, "__FORCED_REFS");
        x.app_build_id = j.at("appBuildId").get<double>();
        x.backup_limit = j.at("backupLimit").get<int64_t>();
        x.backup_on_save = j.at("backupOnSave").get<bool>();
        x.backup_rel_path = get_stack_optional<std::string>(j, "backupRelPath");
        x.bg_color = j.at("bgColor").get<std::string>();
        x.custom_commands = j.at("customCommands").get<std::vector<LdtkCustomCommand>>();
        x.default_entity_height = j.at("defaultEntityHeight").get<int64_t>();
        x.default_entity_width = j.at("defaultEntityWidth").get<int64_t>();
        x.default_grid_size = j.at("defaultGridSize").get<int64_t>();
        x.default_level_bg_color = j.at("defaultLevelBgColor").get<std::string>();
        x.default_level_height = get_stack_optional<int64_t>(j, "defaultLevelHeight");
        x.default_level_width = get_stack_optional<int64_t>(j, "defaultLevelWidth");
        x.default_pivot_x = j.at("defaultPivotX").get<double>();
        x.default_pivot_y = j.at("defaultPivotY").get<double>();
        x.defs = j.at("defs").get<Definitions>();
        x.dummy_world_iid = j.at("dummyWorldIid").get<std::string>();
        x.export_level_bg = j.at("exportLevelBg").get<bool>();
        x.export_png = get_stack_optional<bool>(j, "exportPng");
        x.export_tiled = j.at("exportTiled").get<bool>();
        x.external_levels = j.at("externalLevels").get<bool>();
        x.flags = j.at("flags").get<std::vector<Flag>>();
        x.identifier_style = j.at("identifierStyle").get<IdentifierStyle>();
        x.iid = j.at("iid").get<std::string>();
        x.image_export_mode = j.at("imageExportMode").get<ImageExportMode>();
        x.json_version = j.at("jsonVersion").get<std::string>();
        x.level_name_pattern = j.at("levelNamePattern").get<std::string>();
        x.levels = j.at("levels").get<std::vector<Level>>();
        x.minify_json = j.at("minifyJson").get<bool>();
        x.next_uid = j.at("nextUid").get<int64_t>();
        x.png_file_pattern = get_stack_optional<std::string>(j, "pngFilePattern");
        x.simplified_export = j.at("simplifiedExport").get<bool>();
        x.toc = j.at("toc").get<std::vector<LdtkTableOfContentEntry>>();
        x.tutorial_desc = get_stack_optional<std::string>(j, "tutorialDesc");
        x.world_grid_height = get_stack_optional<int64_t>(j, "worldGridHeight");
        x.world_grid_width = get_stack_optional<int64_t>(j, "worldGridWidth");
        x.world_layout = get_stack_optional<WorldLayout>(j, "worldLayout");
        x.worlds = j.at("worlds").get<std::vector<World>>();
    }

    inline void to_json(json & j, const Ldtk & x) {
        j = json::object();
        j["__FORCED_REFS"] = x.forced_refs;
        j["appBuildId"] = x.app_build_id;
        j["backupLimit"] = x.backup_limit;
        j["backupOnSave"] = x.backup_on_save;
        j["backupRelPath"] = x.backup_rel_path;
        j["bgColor"] = x.bg_color;
        j["customCommands"] = x.custom_commands;
        j["defaultEntityHeight"] = x.default_entity_height;
        j["defaultEntityWidth"] = x.default_entity_width;
        j["defaultGridSize"] = x.default_grid_size;
        j["defaultLevelBgColor"] = x.default_level_bg_color;
        j["defaultLevelHeight"] = x.default_level_height;
        j["defaultLevelWidth"] = x.default_level_width;
        j["defaultPivotX"] = x.default_pivot_x;
        j["defaultPivotY"] = x.default_pivot_y;
        j["defs"] = x.defs;
        j["dummyWorldIid"] = x.dummy_world_iid;
        j["exportLevelBg"] = x.export_level_bg;
        j["exportPng"] = x.export_png;
        j["exportTiled"] = x.export_tiled;
        j["externalLevels"] = x.external_levels;
        j["flags"] = x.flags;
        j["identifierStyle"] = x.identifier_style;
        j["iid"] = x.iid;
        j["imageExportMode"] = x.image_export_mode;
        j["jsonVersion"] = x.json_version;
        j["levelNamePattern"] = x.level_name_pattern;
        j["levels"] = x.levels;
        j["minifyJson"] = x.minify_json;
        j["nextUid"] = x.next_uid;
        j["pngFilePattern"] = x.png_file_pattern;
        j["simplifiedExport"] = x.simplified_export;
        j["toc"] = x.toc;
        j["tutorialDesc"] = x.tutorial_desc;
        j["worldGridHeight"] = x.world_grid_height;
        j["worldGridWidth"] = x.world_grid_width;
        j["worldLayout"] = x.world_layout;
        j["worlds"] = x.worlds;
    }

    inline void from_json(const json & j, When & x) {
        if (j == "AfterLoad") x = When::AFTER_LOAD;
        else if (j == "AfterSave") x = When::AFTER_SAVE;
        else if (j == "BeforeSave") x = When::BEFORE_SAVE;
        else if (j == "Manual") x = When::MANUAL;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const When & x) {
        switch (x) {
            case When::AFTER_LOAD: j = "AfterLoad"; break;
            case When::AFTER_SAVE: j = "AfterSave"; break;
            case When::BEFORE_SAVE: j = "BeforeSave"; break;
            case When::MANUAL: j = "Manual"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, AllowedRefs & x) {
        if (j == "Any") x = AllowedRefs::ANY;
        else if (j == "OnlySame") x = AllowedRefs::ONLY_SAME;
        else if (j == "OnlySpecificEntity") x = AllowedRefs::ONLY_SPECIFIC_ENTITY;
        else if (j == "OnlyTags") x = AllowedRefs::ONLY_TAGS;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const AllowedRefs & x) {
        switch (x) {
            case AllowedRefs::ANY: j = "Any"; break;
            case AllowedRefs::ONLY_SAME: j = "OnlySame"; break;
            case AllowedRefs::ONLY_SPECIFIC_ENTITY: j = "OnlySpecificEntity"; break;
            case AllowedRefs::ONLY_TAGS: j = "OnlyTags"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, EditorDisplayMode & x) {
        if (j == "ArrayCountNoLabel") x = EditorDisplayMode::ARRAY_COUNT_NO_LABEL;
        else if (j == "ArrayCountWithLabel") x = EditorDisplayMode::ARRAY_COUNT_WITH_LABEL;
        else if (j == "EntityTile") x = EditorDisplayMode::ENTITY_TILE;
        else if (j == "Hidden") x = EditorDisplayMode::HIDDEN;
        else if (j == "LevelTile") x = EditorDisplayMode::LEVEL_TILE;
        else if (j == "NameAndValue") x = EditorDisplayMode::NAME_AND_VALUE;
        else if (j == "Points") x = EditorDisplayMode::POINTS;
        else if (j == "PointPath") x = EditorDisplayMode::POINT_PATH;
        else if (j == "PointPathLoop") x = EditorDisplayMode::POINT_PATH_LOOP;
        else if (j == "PointStar") x = EditorDisplayMode::POINT_STAR;
        else if (j == "RadiusGrid") x = EditorDisplayMode::RADIUS_GRID;
        else if (j == "RadiusPx") x = EditorDisplayMode::RADIUS_PX;
        else if (j == "RefLinkBetweenCenters") x = EditorDisplayMode::REF_LINK_BETWEEN_CENTERS;
        else if (j == "RefLinkBetweenPivots") x = EditorDisplayMode::REF_LINK_BETWEEN_PIVOTS;
        else if (j == "ValueOnly") x = EditorDisplayMode::VALUE_ONLY;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const EditorDisplayMode & x) {
        switch (x) {
            case EditorDisplayMode::ARRAY_COUNT_NO_LABEL: j = "ArrayCountNoLabel"; break;
            case EditorDisplayMode::ARRAY_COUNT_WITH_LABEL: j = "ArrayCountWithLabel"; break;
            case EditorDisplayMode::ENTITY_TILE: j = "EntityTile"; break;
            case EditorDisplayMode::HIDDEN: j = "Hidden"; break;
            case EditorDisplayMode::LEVEL_TILE: j = "LevelTile"; break;
            case EditorDisplayMode::NAME_AND_VALUE: j = "NameAndValue"; break;
            case EditorDisplayMode::POINTS: j = "Points"; break;
            case EditorDisplayMode::POINT_PATH: j = "PointPath"; break;
            case EditorDisplayMode::POINT_PATH_LOOP: j = "PointPathLoop"; break;
            case EditorDisplayMode::POINT_STAR: j = "PointStar"; break;
            case EditorDisplayMode::RADIUS_GRID: j = "RadiusGrid"; break;
            case EditorDisplayMode::RADIUS_PX: j = "RadiusPx"; break;
            case EditorDisplayMode::REF_LINK_BETWEEN_CENTERS: j = "RefLinkBetweenCenters"; break;
            case EditorDisplayMode::REF_LINK_BETWEEN_PIVOTS: j = "RefLinkBetweenPivots"; break;
            case EditorDisplayMode::VALUE_ONLY: j = "ValueOnly"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, EditorDisplayPos & x) {
        if (j == "Above") x = EditorDisplayPos::ABOVE;
        else if (j == "Beneath") x = EditorDisplayPos::BENEATH;
        else if (j == "Center") x = EditorDisplayPos::CENTER;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const EditorDisplayPos & x) {
        switch (x) {
            case EditorDisplayPos::ABOVE: j = "Above"; break;
            case EditorDisplayPos::BENEATH: j = "Beneath"; break;
            case EditorDisplayPos::CENTER: j = "Center"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, EditorLinkStyle & x) {
        if (j == "ArrowsLine") x = EditorLinkStyle::ARROWS_LINE;
        else if (j == "CurvedArrow") x = EditorLinkStyle::CURVED_ARROW;
        else if (j == "DashedLine") x = EditorLinkStyle::DASHED_LINE;
        else if (j == "StraightArrow") x = EditorLinkStyle::STRAIGHT_ARROW;
        else if (j == "ZigZag") x = EditorLinkStyle::ZIG_ZAG;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const EditorLinkStyle & x) {
        switch (x) {
            case EditorLinkStyle::ARROWS_LINE: j = "ArrowsLine"; break;
            case EditorLinkStyle::CURVED_ARROW: j = "CurvedArrow"; break;
            case EditorLinkStyle::DASHED_LINE: j = "DashedLine"; break;
            case EditorLinkStyle::STRAIGHT_ARROW: j = "StraightArrow"; break;
            case EditorLinkStyle::ZIG_ZAG: j = "ZigZag"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, TextLanguageMode & x) {
        if (j == "LangC") x = TextLanguageMode::LANG_C;
        else if (j == "LangHaxe") x = TextLanguageMode::LANG_HAXE;
        else if (j == "LangJS") x = TextLanguageMode::LANG_JS;
        else if (j == "LangJson") x = TextLanguageMode::LANG_JSON;
        else if (j == "LangLog") x = TextLanguageMode::LANG_LOG;
        else if (j == "LangLua") x = TextLanguageMode::LANG_LUA;
        else if (j == "LangMarkdown") x = TextLanguageMode::LANG_MARKDOWN;
        else if (j == "LangPython") x = TextLanguageMode::LANG_PYTHON;
        else if (j == "LangRuby") x = TextLanguageMode::LANG_RUBY;
        else if (j == "LangXml") x = TextLanguageMode::LANG_XML;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const TextLanguageMode & x) {
        switch (x) {
            case TextLanguageMode::LANG_C: j = "LangC"; break;
            case TextLanguageMode::LANG_HAXE: j = "LangHaxe"; break;
            case TextLanguageMode::LANG_JS: j = "LangJS"; break;
            case TextLanguageMode::LANG_JSON: j = "LangJson"; break;
            case TextLanguageMode::LANG_LOG: j = "LangLog"; break;
            case TextLanguageMode::LANG_LUA: j = "LangLua"; break;
            case TextLanguageMode::LANG_MARKDOWN: j = "LangMarkdown"; break;
            case TextLanguageMode::LANG_PYTHON: j = "LangPython"; break;
            case TextLanguageMode::LANG_RUBY: j = "LangRuby"; break;
            case TextLanguageMode::LANG_XML: j = "LangXml"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, LimitBehavior & x) {
        if (j == "DiscardOldOnes") x = LimitBehavior::DISCARD_OLD_ONES;
        else if (j == "MoveLastOne") x = LimitBehavior::MOVE_LAST_ONE;
        else if (j == "PreventAdding") x = LimitBehavior::PREVENT_ADDING;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const LimitBehavior & x) {
        switch (x) {
            case LimitBehavior::DISCARD_OLD_ONES: j = "DiscardOldOnes"; break;
            case LimitBehavior::MOVE_LAST_ONE: j = "MoveLastOne"; break;
            case LimitBehavior::PREVENT_ADDING: j = "PreventAdding"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, LimitScope & x) {
        if (j == "PerLayer") x = LimitScope::PER_LAYER;
        else if (j == "PerLevel") x = LimitScope::PER_LEVEL;
        else if (j == "PerWorld") x = LimitScope::PER_WORLD;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const LimitScope & x) {
        switch (x) {
            case LimitScope::PER_LAYER: j = "PerLayer"; break;
            case LimitScope::PER_LEVEL: j = "PerLevel"; break;
            case LimitScope::PER_WORLD: j = "PerWorld"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, RenderMode & x) {
        if (j == "Cross") x = RenderMode::CROSS;
        else if (j == "Ellipse") x = RenderMode::ELLIPSE;
        else if (j == "Rectangle") x = RenderMode::RECTANGLE;
        else if (j == "Tile") x = RenderMode::TILE;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const RenderMode & x) {
        switch (x) {
            case RenderMode::CROSS: j = "Cross"; break;
            case RenderMode::ELLIPSE: j = "Ellipse"; break;
            case RenderMode::RECTANGLE: j = "Rectangle"; break;
            case RenderMode::TILE: j = "Tile"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, TileRenderMode & x) {
        if (j == "Cover") x = TileRenderMode::COVER;
        else if (j == "FitInside") x = TileRenderMode::FIT_INSIDE;
        else if (j == "FullSizeCropped") x = TileRenderMode::FULL_SIZE_CROPPED;
        else if (j == "FullSizeUncropped") x = TileRenderMode::FULL_SIZE_UNCROPPED;
        else if (j == "NineSlice") x = TileRenderMode::NINE_SLICE;
        else if (j == "Repeat") x = TileRenderMode::REPEAT;
        else if (j == "Stretch") x = TileRenderMode::STRETCH;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const TileRenderMode & x) {
        switch (x) {
            case TileRenderMode::COVER: j = "Cover"; break;
            case TileRenderMode::FIT_INSIDE: j = "FitInside"; break;
            case TileRenderMode::FULL_SIZE_CROPPED: j = "FullSizeCropped"; break;
            case TileRenderMode::FULL_SIZE_UNCROPPED: j = "FullSizeUncropped"; break;
            case TileRenderMode::NINE_SLICE: j = "NineSlice"; break;
            case TileRenderMode::REPEAT: j = "Repeat"; break;
            case TileRenderMode::STRETCH: j = "Stretch"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, Checker & x) {
        if (j == "Horizontal") x = Checker::HORIZONTAL;
        else if (j == "None") x = Checker::NONE;
        else if (j == "Vertical") x = Checker::VERTICAL;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const Checker & x) {
        switch (x) {
            case Checker::HORIZONTAL: j = "Horizontal"; break;
            case Checker::NONE: j = "None"; break;
            case Checker::VERTICAL: j = "Vertical"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, TileMode & x) {
        if (j == "Single") x = TileMode::SINGLE;
        else if (j == "Stamp") x = TileMode::STAMP;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const TileMode & x) {
        switch (x) {
            case TileMode::SINGLE: j = "Single"; break;
            case TileMode::STAMP: j = "Stamp"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, Type & x) {
        if (j == "AutoLayer") x = Type::AUTO_LAYER;
        else if (j == "Entities") x = Type::ENTITIES;
        else if (j == "IntGrid") x = Type::INT_GRID;
        else if (j == "Tiles") x = Type::TILES;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const Type & x) {
        switch (x) {
            case Type::AUTO_LAYER: j = "AutoLayer"; break;
            case Type::ENTITIES: j = "Entities"; break;
            case Type::INT_GRID: j = "IntGrid"; break;
            case Type::TILES: j = "Tiles"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, EmbedAtlas & x) {
        if (j == "LdtkIcons") x = EmbedAtlas::LDTK_ICONS;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const EmbedAtlas & x) {
        switch (x) {
            case EmbedAtlas::LDTK_ICONS: j = "LdtkIcons"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, Flag & x) {
        if (j == "DiscardPreCsvIntGrid") x = Flag::DISCARD_PRE_CSV_INT_GRID;
        else if (j == "ExportOldTableOfContentData") x = Flag::EXPORT_OLD_TABLE_OF_CONTENT_DATA;
        else if (j == "ExportPreCsvIntGridFormat") x = Flag::EXPORT_PRE_CSV_INT_GRID_FORMAT;
        else if (j == "IgnoreBackupSuggest") x = Flag::IGNORE_BACKUP_SUGGEST;
        else if (j == "MultiWorlds") x = Flag::MULTI_WORLDS;
        else if (j == "PrependIndexToLevelFileNames") x = Flag::PREPEND_INDEX_TO_LEVEL_FILE_NAMES;
        else if (j == "UseMultilinesType") x = Flag::USE_MULTILINES_TYPE;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const Flag & x) {
        switch (x) {
            case Flag::DISCARD_PRE_CSV_INT_GRID: j = "DiscardPreCsvIntGrid"; break;
            case Flag::EXPORT_OLD_TABLE_OF_CONTENT_DATA: j = "ExportOldTableOfContentData"; break;
            case Flag::EXPORT_PRE_CSV_INT_GRID_FORMAT: j = "ExportPreCsvIntGridFormat"; break;
            case Flag::IGNORE_BACKUP_SUGGEST: j = "IgnoreBackupSuggest"; break;
            case Flag::MULTI_WORLDS: j = "MultiWorlds"; break;
            case Flag::PREPEND_INDEX_TO_LEVEL_FILE_NAMES: j = "PrependIndexToLevelFileNames"; break;
            case Flag::USE_MULTILINES_TYPE: j = "UseMultilinesType"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, BgPos & x) {
        if (j == "Contain") x = BgPos::CONTAIN;
        else if (j == "Cover") x = BgPos::COVER;
        else if (j == "CoverDirty") x = BgPos::COVER_DIRTY;
        else if (j == "Repeat") x = BgPos::REPEAT;
        else if (j == "Unscaled") x = BgPos::UNSCALED;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const BgPos & x) {
        switch (x) {
            case BgPos::CONTAIN: j = "Contain"; break;
            case BgPos::COVER: j = "Cover"; break;
            case BgPos::COVER_DIRTY: j = "CoverDirty"; break;
            case BgPos::REPEAT: j = "Repeat"; break;
            case BgPos::UNSCALED: j = "Unscaled"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, WorldLayout & x) {
        if (j == "Free") x = WorldLayout::FREE;
        else if (j == "GridVania") x = WorldLayout::GRID_VANIA;
        else if (j == "LinearHorizontal") x = WorldLayout::LINEAR_HORIZONTAL;
        else if (j == "LinearVertical") x = WorldLayout::LINEAR_VERTICAL;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const WorldLayout & x) {
        switch (x) {
            case WorldLayout::FREE: j = "Free"; break;
            case WorldLayout::GRID_VANIA: j = "GridVania"; break;
            case WorldLayout::LINEAR_HORIZONTAL: j = "LinearHorizontal"; break;
            case WorldLayout::LINEAR_VERTICAL: j = "LinearVertical"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, IdentifierStyle & x) {
        if (j == "Capitalize") x = IdentifierStyle::CAPITALIZE;
        else if (j == "Free") x = IdentifierStyle::FREE;
        else if (j == "Lowercase") x = IdentifierStyle::LOWERCASE;
        else if (j == "Uppercase") x = IdentifierStyle::UPPERCASE;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const IdentifierStyle & x) {
        switch (x) {
            case IdentifierStyle::CAPITALIZE: j = "Capitalize"; break;
            case IdentifierStyle::FREE: j = "Free"; break;
            case IdentifierStyle::LOWERCASE: j = "Lowercase"; break;
            case IdentifierStyle::UPPERCASE: j = "Uppercase"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }

    inline void from_json(const json & j, ImageExportMode & x) {
        if (j == "LayersAndLevels") x = ImageExportMode::LAYERS_AND_LEVELS;
        else if (j == "None") x = ImageExportMode::NONE;
        else if (j == "OneImagePerLayer") x = ImageExportMode::ONE_IMAGE_PER_LAYER;
        else if (j == "OneImagePerLevel") x = ImageExportMode::ONE_IMAGE_PER_LEVEL;
        else { throw std::runtime_error("Input JSON does not conform to schema!"); }
    }

    inline void to_json(json & j, const ImageExportMode & x) {
        switch (x) {
            case ImageExportMode::LAYERS_AND_LEVELS: j = "LayersAndLevels"; break;
            case ImageExportMode::NONE: j = "None"; break;
            case ImageExportMode::ONE_IMAGE_PER_LAYER: j = "OneImagePerLayer"; break;
            case ImageExportMode::ONE_IMAGE_PER_LEVEL: j = "OneImagePerLevel"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"[object Object]\": " + std::to_string(static_cast<int>(x)));
        }
    }
}
