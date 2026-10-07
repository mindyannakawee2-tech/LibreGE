package librege.host;

import java.io.*;
import java.nio.file.*;

public final class Main {

    private static String detectPlatform() {
        String os =
            System
                .getProperty("os.name")
                .toLowerCase();

        if (os.contains("win")) {
            return "Windows";
        }

        if (
            os.contains("mac")
            || os.contains("darwin")
        ) {
            return "macOS";
        }

        if (os.contains("linux")) {
            return "Linux";
        }

        return "Unknown";
    }

    public static void main(
        String[] args
    ) throws Exception {

        String runtime =
            args.length > 0
                ? args[0]
                : "client";

        String platform =
            args.length > 1
                ? args[1]
                : detectPlatform();

        Path pluginDirectory =
            Paths.get("plugins");

        System.out.println(
            "[LibrePluginHost] Runtime  : "
            + runtime
        );

        System.out.println(
            "[LibrePluginHost] Platform : "
            + platform
        );

        PluginLoader loader =
            new PluginLoader(
                runtime,
                platform
            );

        loader.loadDirectory(
            pluginDirectory
        );

        BufferedReader input =
            new BufferedReader(
                new InputStreamReader(
                    System.in
                )
            );

        String line;

        while (
            (line = input.readLine())
            != null
        ) {
            line = line.trim();

            if (
                line.equalsIgnoreCase(
                    "SHUTDOWN"
                )
            ) {
                break;
            }

            if (
                line.startsWith(
                    "TICK "
                )
            ) {
                try {
                    double dt =
                        Double.parseDouble(
                            line.substring(5)
                        );

                    loader.update(dt);
                }
                catch (
                    NumberFormatException exception
                ) {
                    System.err.println(
                        "[LibrePluginHost] Invalid TICK"
                    );
                }
            }
        }

        loader.unloadAll();

        System.out.println(
            "[LibrePluginHost] Shutdown complete"
        );
    }
}
