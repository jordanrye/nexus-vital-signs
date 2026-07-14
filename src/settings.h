#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>

namespace Settings {
    
    void LoadAllLayouts();
    void SaveAllLayouts();
    bool IsDirtyLayouts();
    
    void LoadPresets();
    void SavePresets();
    bool IsDirtyPresets();

} // namespace Settings

#endif /* SETTINGS_H */