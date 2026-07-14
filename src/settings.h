#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>

namespace Settings {
    
    void LoadSettings();
    void SaveSettings();
    bool IsDirtySettings();

    void LoadAllLayouts();
    void SaveAllLayouts();
    bool IsDirtyLayouts();
    
    void LoadPresets();
    void SavePresets();
    bool IsDirtyPresets();

} // namespace Settings

#endif /* SETTINGS_H */