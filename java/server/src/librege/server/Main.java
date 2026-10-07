package librege.server;

import javax.swing.*;
import javax.swing.border.EmptyBorder;

import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.net.*;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public final class Main {

    public static void main(
        String[] args
    ) {
        SwingUtilities.invokeLater(
            () -> new ServerWindow()
        );
    }
}

final class PlayerState {

    final int id;

    String name;

    volatile float x = 300.0f;
    volatile float y = 200.0f;

    ClientConnection connection;

    PlayerState(
        int id,
        String name
    ) {
        this.id = id;
        this.name = name;
    }

    @Override
    public String toString() {
        return
            name
            + " ["
            + id
            + "]";
    }
}

final class GameServer {

    private final ServerWindow ui;

    private ServerSocket socket;

    private volatile boolean running;

    private final AtomicInteger nextID =
        new AtomicInteger(1);

    private final ConcurrentHashMap<
        Integer,
        PlayerState
    > players =
        new ConcurrentHashMap<>();

    private final ExecutorService clients =
        Executors.newCachedThreadPool();

    GameServer(
        ServerWindow ui
    ) {
        this.ui = ui;
    }

    synchronized void start(
        int port
    ) throws IOException {

        if (running) {
            return;
        }

        socket =
            new ServerSocket(port);

        running = true;

        ui.log(
            "Starting LibreGE test server..."
        );

        ui.log(
            "Listening on port "
            + port
        );

        Thread acceptThread =
            new Thread(
                this::acceptLoop,
                "LibreGE-Accept"
            );

        acceptThread.setDaemon(
            true
        );

        acceptThread.start();
    }

    private void acceptLoop() {
        while (running) {
            try {
                Socket client =
                    socket.accept();

                clients.submit(
                    () -> handleClient(
                        client
                    )
                );
            }
            catch (IOException e) {
                if (running) {
                    ui.log(
                        "Accept error: "
                        + e.getMessage()
                    );
                }
            }
        }
    }

    private void handleClient(
        Socket socket
    ) {
        PlayerState player =
            null;

        try (
            BufferedReader input =
                new BufferedReader(
                    new InputStreamReader(
                        socket.getInputStream()
                    )
                );

            PrintWriter output =
                new PrintWriter(
                    new BufferedWriter(
                        new OutputStreamWriter(
                            socket.getOutputStream()
                        )
                    ),
                    true
                )
        ) {
            String hello =
                input.readLine();

            if (
                hello == null ||
                !hello.startsWith(
                    "HELLO "
                )
            ) {
                return;
            }

            String name =
                hello.substring(6)
                    .trim()
                    .replace(
                        " ",
                        "_"
                    );

            if (name.isBlank()) {
                name =
                    "Player";
            }

            int id =
                nextID.getAndIncrement();

            player =
                new PlayerState(
                    id,
                    name
                );

            player.connection =
                new ClientConnection(
                    socket,
                    output
                );

            players.put(
                id,
                player
            );

            output.println(
                "WELCOME "
                + id
            );

            /*
             * Tell the new client about
             * everybody already online.
             */
            for (
                PlayerState existing :
                players.values()
            ) {
                sendPlayer(
                    output,
                    existing
                );
            }

            broadcastPlayer(
                player
            );

            ui.log(
                name
                + " joined the server"
            );

            ui.refreshPlayers(
                players.values()
            );

            String line;

            while (
                running &&
                (
                    line =
                        input.readLine()
                )
                != null
            ) {
                handlePacket(
                    player,
                    line
                );
            }
        }
        catch (IOException e) {
            if (running) {
                ui.log(
                    "Client error: "
                    + e.getMessage()
                );
            }
        }
        finally {
            if (player != null) {
                players.remove(
                    player.id
                );

                broadcast(
                    "REMOVE "
                    + player.id
                );

                ui.log(
                    player.name
                    + " left the server"
                );

                ui.refreshPlayers(
                    players.values()
                );
            }

            try {
                socket.close();
            }
            catch (IOException ignored) {
            }
        }
    }

    private void handlePacket(
        PlayerState player,
        String line
    ) {
        String[] parts =
            line.split(
                "\\s+"
            );

        if (
            parts.length == 3 &&
            parts[0].equals(
                "POS"
            )
        ) {
            try {
                player.x =
                    Float.parseFloat(
                        parts[1]
                    );

                player.y =
                    Float.parseFloat(
                        parts[2]
                    );

                broadcastPlayer(
                    player
                );
            }
            catch (
                NumberFormatException ignored
            ) {
            }
        }
    }

    private void sendPlayer(
        PrintWriter output,
        PlayerState player
    ) {
        output.println(
            "PLAYER "
            + player.id
            + " "
            + player.name
            + " "
            + player.x
            + " "
            + player.y
        );
    }

    private void broadcastPlayer(
        PlayerState player
    ) {
        broadcast(
            "PLAYER "
            + player.id
            + " "
            + player.name
            + " "
            + player.x
            + " "
            + player.y
        );
    }

    synchronized void broadcast(
        String message
    ) {
        for (
            PlayerState player :
            players.values()
        ) {
            if (
                player.connection
                != null
            ) {
                player.connection.send(
                    message
                );
            }
        }
    }

    void say(
        String message
    ) {
        ui.log(
            "[Server] "
            + message
        );

        broadcast(
            "CHAT Server "
            + message
        );
    }

    void listPlayers() {
        if (players.isEmpty()) {
            ui.log(
                "No players connected."
            );

            return;
        }

        StringBuilder result =
            new StringBuilder(
                "Players: "
            );

        boolean first =
            true;

        for (
            PlayerState player :
            players.values()
        ) {
            if (!first) {
                result.append(", ");
            }

            result.append(
                player.name
            );

            first =
                false;
        }

        ui.log(
            result.toString()
        );
    }

    synchronized void stop() {
        if (!running) {
            return;
        }

        running =
            false;

        ui.log(
            "Stopping server..."
        );

        broadcast(
            "SERVER_STOP"
        );

        for (
            PlayerState player :
            players.values()
        ) {
            if (
                player.connection
                != null
            ) {
                player.connection.close();
            }
        }

        players.clear();

        try {
            socket.close();
        }
        catch (IOException ignored) {
        }

        ui.refreshPlayers(
            players.values()
        );

        ui.log(
            "Server stopped."
        );
    }

    boolean isRunning() {
        return running;
    }
}

final class ClientConnection {

    private final Socket socket;
    private final PrintWriter output;

    ClientConnection(
        Socket socket,
        PrintWriter output
    ) {
        this.socket =
            socket;

        this.output =
            output;
    }

    synchronized void send(
        String message
    ) {
        output.println(
            message
        );
    }

    void close() {
        try {
            socket.close();
        }
        catch (
            IOException ignored
        ) {
        }
    }
}

final class ServerWindow
    extends JFrame {

    private final JTextArea console =
        new JTextArea();

    private final DefaultListModel<
        PlayerState
    > playerModel =
        new DefaultListModel<>();

    private final JList<
        PlayerState
    > playerList =
        new JList<>(
            playerModel
        );

    private final JTextField command =
        new JTextField();

    private final JTextField port =
        new JTextField(
            "25565",
            6
        );

    private final JButton start =
        new JButton(
            "Start Server"
        );

    private final JButton stop =
        new JButton(
            "Stop Server"
        );

    private final JLabel status =
        new JLabel(
            "Stopped"
        );

    private final GameServer server;

    ServerWindow() {
        super(
            "LibreGE SERVER"
        );

        server =
            new GameServer(
                this
            );

        buildUI();

        setSize(
            850,
            550
        );

        setLocationRelativeTo(
            null
        );

        setDefaultCloseOperation(
            WindowConstants.DO_NOTHING_ON_CLOSE
        );

        addWindowListener(
            new WindowAdapter() {
                @Override
                public void windowClosing(
                    WindowEvent event
                ) {
                    server.stop();
                    dispose();
                    System.exit(0);
                }
            }
        );

        setVisible(
            true
        );

        log(
            "LibreGE SERVER 0.1"
        );

        log(
            "Ready."
        );
    }

    private void buildUI() {
        console.setEditable(
            false
        );

        console.setFont(
            new Font(
                Font.MONOSPACED,
                Font.PLAIN,
                13
            )
        );

        JScrollPane consoleScroll =
            new JScrollPane(
                console
            );

        JPanel right =
            new JPanel(
                new BorderLayout(
                    5,
                    5
                )
            );

        right.setBorder(
            new EmptyBorder(
                8,
                8,
                8,
                8
            )
        );

        right.add(
            new JLabel(
                "Players"
            ),
            BorderLayout.NORTH
        );

        right.add(
            new JScrollPane(
                playerList
            ),
            BorderLayout.CENTER
        );

        JPanel top =
            new JPanel(
                new FlowLayout(
                    FlowLayout.LEFT
                )
            );

        top.add(
            new JLabel(
                "Port:"
            )
        );

        top.add(
            port
        );

        top.add(
            start
        );

        top.add(
            stop
        );

        top.add(
            status
        );

        JPanel bottom =
            new JPanel(
                new BorderLayout(
                    5,
                    5
                )
            );

        bottom.add(
            new JLabel(
                "Command:"
            ),
            BorderLayout.WEST
        );

        bottom.add(
            command,
            BorderLayout.CENTER
        );

        JSplitPane split =
            new JSplitPane(
                JSplitPane.HORIZONTAL_SPLIT,
                consoleScroll,
                right
            );

        split.setResizeWeight(
            0.78
        );

        setLayout(
            new BorderLayout()
        );

        add(
            top,
            BorderLayout.NORTH
        );

        add(
            split,
            BorderLayout.CENTER
        );

        add(
            bottom,
            BorderLayout.SOUTH
        );

        stop.setEnabled(
            false
        );

        start.addActionListener(
            event ->
                startServer()
        );

        stop.addActionListener(
            event ->
                stopServer()
        );

        command.addActionListener(
            event ->
                executeCommand()
        );
    }

    private void startServer() {
        try {
            int selectedPort =
                Integer.parseInt(
                    port.getText().trim()
                );

            server.start(
                selectedPort
            );

            start.setEnabled(
                false
            );

            stop.setEnabled(
                true
            );

            port.setEnabled(
                false
            );

            status.setText(
                "Running"
            );
        }
        catch (Exception exception) {
            log(
                "Failed to start: "
                + exception.getMessage()
            );
        }
    }

    private void stopServer() {
        server.stop();

        start.setEnabled(
            true
        );

        stop.setEnabled(
            false
        );

        port.setEnabled(
            true
        );

        status.setText(
            "Stopped"
        );
    }

    private void executeCommand() {
        String text =
            command
                .getText()
                .trim();

        command.setText(
            ""
        );

        if (text.isEmpty()) {
            return;
        }

        log(
            "> "
            + text
        );

        if (
            text.equalsIgnoreCase(
                "stop"
            )
        ) {
            stopServer();
            return;
        }

        if (
            text.equalsIgnoreCase(
                "list"
            )
        ) {
            server.listPlayers();
            return;
        }

        if (
            text.startsWith(
                "say "
            )
        ) {
            server.say(
                text.substring(4)
            );

            return;
        }

        log(
            "Unknown command."
        );

        log(
            "Commands: list, say <message>, stop"
        );
    }

    void log(
        String message
    ) {
        SwingUtilities.invokeLater(
            () -> {
                console.append(
                    message
                    + "\n"
                );

                console.setCaretPosition(
                    console
                        .getDocument()
                        .getLength()
                );
            }
        );
    }

    void refreshPlayers(
        Collection<PlayerState> players
    ) {
        SwingUtilities.invokeLater(
            () -> {
                playerModel.clear();

                players
                    .stream()
                    .sorted(
                        Comparator.comparingInt(
                            player ->
                                player.id
                        )
                    )
                    .forEach(
                        playerModel::addElement
                    );
            }
        );
    }
}
