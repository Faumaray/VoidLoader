package ru.faumaray;

import java.io.File;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Files;
import java.nio.file.Path;

public class VoidLoaderMain {
    public static void main(String[] args) {
        System.out.println("[VoidLoader] Custom loader starting...");

        // Find the real Minecraft jar near this one
        Path baseDir = Path.of(System.getProperty("user.dir"));
        Path versionsDir = baseDir.resolve("versions");
        Path mcJar = null;

        try {
            // Try to guess version folder (e.g. .minecraft/versions/1.20.1/1.20.1.jar)
            for (Path version : Files.newDirectoryStream(versionsDir)) {
                Path candidate = version.resolve(version.getFileName() + ".jar");
                if (Files.exists(candidate)) {
                    mcJar = candidate;
                    break;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (mcJar == null || !Files.exists(mcJar)) {
            System.err.println("[VoidLoader] Could not locate real Minecraft jar!");
            return;
        }

        System.out.println("[VoidLoader] Loading Minecraft from " + mcJar);

        try (URLClassLoader mcLoader = new URLClassLoader(
                new URL[]{ mcJar.toUri().toURL() },
                VoidLoaderMain.class.getClassLoader())) {

            Class<?> mcMain = Class.forName("net.minecraft.client.main.Main", true, mcLoader);
            Method mainMethod = mcMain.getMethod("main", String[].class);

            System.out.println("[VoidLoader] Launching Minecraft main...");
            mainMethod.invoke(null, (Object) args);

        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
