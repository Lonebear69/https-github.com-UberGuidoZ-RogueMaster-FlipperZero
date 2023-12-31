#include "lfs_backup.h"

#include <toolbox/tar/tar_archive.h>

#include <notification/notification_settings_filename.h>

#define LFS_BACKUP_DEFAULT_LOCATION EXT_PATH(LFS_BACKUP_DEFAULT_FILENAME)

static void backup_name_converter(FuriString* filename) {
    if(furi_string_empty(filename) || (furi_string_get_char(filename, 0) == '.')) {
        return;
    }

    /* Filenames are already prefixed with '.' */
    const char* const names[] = {
        NOTIFICATION_SETTINGS_FILE_NAME,
    };

    for(size_t i = 0; i < COUNT_OF(names); i++) {
        if(furi_string_equal(filename, &names[i][1])) {
            furi_string_set(filename, names[i]);
            return;
        }
    }
}

bool lfs_backup_create(Storage* storage, const char* destination) {
    const char* final_destination =
        destination && strlen(destination) ? destination : LFS_BACKUP_DEFAULT_LOCATION;
    return storage_int_backup(storage, final_destination) == FSE_OK;
}

bool lfs_backup_exists(Storage* storage, const char* source) {
    const char* final_source = source && strlen(source) ? source : LFS_BACKUP_DEFAULT_LOCATION;
    return storage_common_stat(storage, final_source, NULL) == FSE_OK;
}

bool lfs_backup_unpack(Storage* storage, const char* source) {
    const char* final_source = source && strlen(source) ? source : LFS_BACKUP_DEFAULT_LOCATION;
    return storage_int_restore(storage, final_source, backup_name_converter) == FSE_OK;
}
