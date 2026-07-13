#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>

namespace Settings {
    
    void LoadAllLayouts();
    void SaveAllLayouts();
    
    void LoadPresets();
    void SavePresets();

} // namespace Settings

#endif /* SETTINGS_H */