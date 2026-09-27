#include "textobj.h"

#include <stdio.h>
#include <stdlib.h>

#include "safe.h"
#include "text_loader.h"

static void mode_room_add_textobj_area(mode_room_impl_t *impl, int32_t x,
                                       int32_t y, int32_t width, int32_t height,
                                       int32_t text_id) {
    object_t *object;

    if (!impl || !impl->controller || !impl->controller->object) {
        return;
    }

    (void)text_id;

    object = smalloc(sizeof(*object));
    object_init(object);

    object_set_position(object, x, y);
    object_set_size(object, width, height);
    object_set_visible(object, false);

    object_layer_add_object(impl->controller->object, object);
}

void mode_room_add_text_obj(mode_room_impl_t *impl, int32_t type_or_flag,
                            int32_t x, int32_t y, int32_t width, int32_t height,
                            int32_t text_id, int32_t param7) {
    (void)type_or_flag;
    (void)param7;

    if (!impl || !impl->controller) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    mode_room_add_textobj_area(impl, x, y, width, height, text_id);
}
