/**
 * @file      app_registry.c
 * @brief     Application registry and configuration parser
 * @author    T-Lora-Pager-SkinnyCon Team
 * @license   MIT
 */

#include "app_registry.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Initialize the app registry with default configuration
 * @param registry Pointer to the registry structure
 * @return 0 on success, -1 on failure
 */
int app_registry_init(AppRegistry *registry) {
    if (!registry) {
        return -1;
    }
    
    registry->app_count = 0;
    registry->max_apps = APP_REGISTRY_MAX_APPS;
    registry->factory_enabled = true;
    registry->native_build = false;
    
    memset(registry->apps, 0, sizeof(registry->apps));
    
    return 0;
}

/**
 * @brief Parse platformio.ini configuration file
 * @param config_path Path to the platformio.ini file
 * @param registry Pointer to the registry structure
 * @return 0 on success, -1 on failure
 */
int app_registry_parse_config(const char *config_path, AppRegistry *registry) {
    if (!config_path || !registry) {
        return -1;
    }
    
    FILE *file = fopen(config_path, "r");
    if (!file) {
        return -1;
    }
    
    char line[256];
    int in_platformio_section = 0;
    int in_build_section = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Check for section headers
        if (line[0] == '[') {
            in_platformio_section = (strstr(line, "[platformio]") != NULL);
            in_build_section = (strstr(line, "[env:") != NULL);
            continue;
        }
        
        // Parse build_src_filter to detect factory app
        if (in_build_section && strstr(line, "build_src_filter") != NULL) {
            if (strstr(line, "factory") != NULL || strstr(line, "examples/factory") != NULL) {
                registry->factory_enabled = true;
            } else {
                registry->factory_enabled = false;
            }
        }
        
        // Parse build_flags for native build detection
        if (in_build_section && strstr(line, "build_flags") != NULL) {
            if (strstr(line, "NATIVE_BUILD") != NULL) {
                registry->native_build = true;
            }
        }
    }
    
    fclose(file);
    return 0;
}

/**
 * @brief Register an application in the registry
 * @param registry Pointer to the registry structure
 * @param app_name Name of the application
 * @param app_path Path to the application source
 * @param is_factory Whether this is a factory app
 * @return 0 on success, -1 on failure
 */
int app_registry_register_app(AppRegistry *registry, const char *app_name, 
                              const char *app_path, bool is_factory) {
    if (!registry || !app_name || !app_path) {
        return -1;
    }
    
    if (registry->app_count >= registry->max_apps) {
        return -1;
    }
    
    AppEntry *entry = &registry->apps[registry->app_count];
    strncpy(entry->name, app_name, APP_NAME_MAX_LEN - 1);
    entry->name[APP_NAME_MAX_LEN - 1] = '\0';
    strncpy(entry->path, app_path, APP_PATH_MAX_LEN - 1);
    entry->path[APP_PATH_MAX_LEN - 1] = '\0';
    entry->is_factory = is_factory;
    entry->enabled = true;
    
    registry->app_count++;
    return 0;
}

/**
 * @brief Filter applications based on criteria
 * @param registry Pointer to the registry structure
 * @param filter_type Type of filter to apply
 * @param filter_value Filter value
 * @return Number of matching apps, -1 on error
 */
int app_registry_filter_apps(AppRegistry *registry, FilterType filter_type, 
                             const char *filter_value) {
    if (!registry) {
        return -1;
    }
    
    int match_count = 0;
    
    for (int i = 0; i < registry->app_count; i++) {
        AppEntry *entry = &registry->apps[i];
        bool matches = false;
        
        switch (filter_type) {
            case FILTER_FACTORY:
                matches = (entry->is_factory == (strcmp(filter_value, "true") == 0));
                break;
            case FILTER_NAME:
                matches = (strstr(entry->name, filter_value) != NULL);
                break;
            case FILTER_PATH:
                matches = (strstr(entry->path, filter_value) != NULL);
                break;
            case FILTER_ENABLED:
                matches = entry->enabled;
                break;
            default:
                matches = true;
                break;
        }
        
        if (matches) {
            match_count++;
        }
    }
    
    return match_count;
}

/**
 * @brief Remove factory app from registry
 * @param registry Pointer to the registry structure
 * @return 0 on success, -1 on failure
 */
int app_registry_remove_factory(AppRegistry *registry) {
    if (!registry) {
        return -1;
    }
    
    int new_count = 0;
    
    for (int i = 0; i < registry->app_count; i++) {
        AppEntry *entry = &registry->apps[i];
        if (!entry->is_factory) {
            if (new_count != i) {
                registry->apps[new_count] = *entry;
            }
            new_count++;
        }
    }
    
    registry->app_count = new_count;
    registry->factory_enabled = false;
    
    return 0;
}

/**
 * @brief Get application count by type
 * @param registry Pointer to the registry structure
 * @param type Type of application to count
 * @return Number of applications, -1 on error
 */
int app_registry_get_app_count(AppRegistry *registry, AppType type) {
    if (!registry) {
        return -1;
    }
    
    int count = 0;
    
    for (int i = 0; i < registry->app_count; i++) {
        AppEntry *entry = &registry->apps[i];
        
        switch (type) {
            case APP_TYPE_FACTORY:
                if (entry->is_factory) count++;
                break;
            case APP_TYPE_NON_FACTORY:
                if (!entry->is_factory) count++;
                break;
            case APP_TYPE_ALL:
                count++;
                break;
            default:
                break;
        }
    }
    
    return count;
}

/**
 * @brief Print registry information to stdout
 * @param registry Pointer to the registry structure
 * @return 0 on success, -1 on error
 */
int app_registry_print(AppRegistry *registry) {
    if (!registry) {
        return -1;
    }
    
    printf("=== App Registry ===\n");
    printf("Total apps: %d\n", registry->app_count);
    printf("Factory enabled: %s\n", registry->factory_enabled ? "yes" : "no");
    printf("Native build: %s\n", registry->native_build ? "yes" : "no");
    printf("\nRegistered apps:\n");
    
    for (int i = 0; i < registry->app_count; i++) {
        AppEntry *entry = &registry->apps[i];
        printf("  [%d] %s (%s) - %s\n", 
               i, 
               entry->name, 
               entry->path, 
               entry->is_factory ? "factory" : "non-factory");
    }
    
    return 0;
}
