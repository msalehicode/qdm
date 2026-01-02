// MyJavaClass.java
package org.qtproject.qmldownloadmanager;

import java.io.*;

public class MyJavaClass {
    public static boolean makeDirectory(String path) {
        try {
            File dir = new File(path);
            return dir.exists() || dir.mkdirs();
        } catch (Exception e) { e.printStackTrace(); return false; }
    }

    public static boolean writeBytesFile(String path, String fileName, byte[] data) {
        try {
            File dir = new File(path);
            if (!dir.exists()) dir.mkdirs();
            File file = new File(dir, fileName);

            // true = append mode
            FileOutputStream fos = new FileOutputStream(file, true);
            fos.write(data);
            fos.flush();
            fos.close();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

}

