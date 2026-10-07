package librege.sample;

import librege.api.*;

public final class SamplePlugin
    implements LibrePlugin {

    private double elapsed = 0.0;

    @Override
    public String getName() {
        return "LibreGE Sample Plugin";
    }

    @Override
    public String getVersion() {
        return "0.1.0";
    }

    @Override
    public int getAPIVersion() {
        return LibrePluginAPI.VERSION;
    }

    @Override
    public void onLoad(
        LibreContext context
    ) {
        context.log(
            "SamplePlugin loaded!"
        );

        context.log(
            "Engine "
            + context.getEngineVersion()
        );

        context.log(
            "Platform "
            + context.getPlatform()
        );
    }

    @Override
    public void onUpdate(
        double deltaTime
    ) {
        elapsed += deltaTime;

        if (elapsed >= 5.0) {
            System.out.println(
                "[SamplePlugin] Still alive!"
            );

            elapsed = 0.0;
        }
    }

    @Override
    public void onUnload() {
        System.out.println(
            "[SamplePlugin] Goodbye!"
        );
    }
}
