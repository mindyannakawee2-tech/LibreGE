package librege.api;

public final class LibreContext {

    private final String runtime;
    private final String platform;
    private final String engineVersion;

    public LibreContext(
        String runtime,
        String platform,
        String engineVersion
    ) {
        this.runtime = runtime;
        this.platform = platform;
        this.engineVersion = engineVersion;
    }

    public String getRuntime() {
        return runtime;
    }

    public String getPlatform() {
        return platform;
    }

    public String getEngineVersion() {
        return engineVersion;
    }

    public void log(String message) {
        System.out.println(
            "[Plugin/" + runtime + "] " + message
        );
    }
}
