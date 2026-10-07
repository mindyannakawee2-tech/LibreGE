package librege.host;

import librege.api.LibrePlugin;

import java.net.URLClassLoader;

public final class LoadedPlugin {

    public final LibrePlugin plugin;
    public final URLClassLoader classLoader;

    public LoadedPlugin(
        LibrePlugin plugin,
        URLClassLoader classLoader
    ) {
        this.plugin = plugin;
        this.classLoader = classLoader;
    }
}
