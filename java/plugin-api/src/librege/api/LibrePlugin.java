package librege.api;

public interface LibrePlugin {

    String getName();

    String getVersion();

    int getAPIVersion();

    void onLoad(LibreContext context);

    void onUpdate(double deltaTime);

    void onUnload();
}
