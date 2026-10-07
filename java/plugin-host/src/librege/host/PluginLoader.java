package librege.host;

import librege.api.*;

import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.util.*;
import java.util.jar.*;

public final class PluginLoader {

    private final List<LoadedPlugin> plugins =
        new ArrayList<>();

    private final LibreContext context;

    public PluginLoader(
        String runtime,
        String platform
    ) {
        context = new LibreContext(
            runtime,
            platform,
            "0.1.0"
        );
    }

    public void loadDirectory(Path directory)
        throws IOException {

        if (!Files.exists(directory)) {
            Files.createDirectories(directory);
        }

        try (
            DirectoryStream<Path> stream =
                Files.newDirectoryStream(
                    directory,
                    "*.jar"
                )
        ) {
            for (Path jar : stream) {
                loadPlugin(jar);
            }
        }

        System.out.println(
            "[LibrePluginHost] "
            + plugins.size()
            + " plugin(s) loaded"
        );
    }

    private void loadPlugin(Path jarPath) {
        try {
            JarFile jar =
                new JarFile(
                    jarPath.toFile()
                );

            Manifest manifest =
                jar.getManifest();

            if (manifest == null) {
                System.out.println(
                    "[LibrePluginHost] Skipping "
                    + jarPath
                    + ": no manifest"
                );

                jar.close();
                return;
            }

            String mainClass =
                manifest
                    .getMainAttributes()
                    .getValue(
                        "LibreGE-Plugin-Class"
                    );

            if (mainClass == null) {
                jar.close();
                return;
            }

            URLClassLoader loader =
                new URLClassLoader(
                    new URL[] {
                        jarPath
                            .toUri()
                            .toURL()
                    },
                    LibrePlugin.class
                        .getClassLoader()
                );

            Class<?> type =
                Class.forName(
                    mainClass,
                    true,
                    loader
                );

            Object instance =
                type
                    .getDeclaredConstructor()
                    .newInstance();

            if (!(instance instanceof LibrePlugin)) {
                throw new IllegalStateException(
                    mainClass
                    + " does not implement LibrePlugin"
                );
            }

            LibrePlugin plugin =
                (LibrePlugin) instance;

            if (
                plugin.getAPIVersion()
                !=
                LibrePluginAPI.VERSION
            ) {
                System.out.println(
                    "[LibrePluginHost] API mismatch: "
                    + plugin.getName()
                );

                loader.close();
                jar.close();

                return;
            }

            plugin.onLoad(context);

            plugins.add(
                new LoadedPlugin(
                    plugin,
                    loader
                )
            );

            System.out.println(
                "[LibrePluginHost] Loaded "
                + plugin.getName()
                + " "
                + plugin.getVersion()
            );

            jar.close();
        }
        catch (Exception exception) {
            System.err.println(
                "[LibrePluginHost] Failed loading "
                + jarPath
            );

            exception.printStackTrace();
        }
    }

    public void update(double deltaTime) {
        for (LoadedPlugin loaded : plugins) {
            try {
                loaded.plugin.onUpdate(
                    deltaTime
                );
            }
            catch (Exception exception) {
                System.err.println(
                    "[LibrePluginHost] Plugin update failed: "
                    + loaded.plugin.getName()
                );

                exception.printStackTrace();
            }
        }
    }

    public void unloadAll() {
        ListIterator<LoadedPlugin> iterator =
            plugins.listIterator(
                plugins.size()
            );

        while (iterator.hasPrevious()) {
            LoadedPlugin loaded =
                iterator.previous();

            try {
                loaded.plugin.onUnload();
            }
            catch (Exception exception) {
                exception.printStackTrace();
            }

            try {
                loaded.classLoader.close();
            }
            catch (IOException ignored) {
            }
        }

        plugins.clear();
    }
}
