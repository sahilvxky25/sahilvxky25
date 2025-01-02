import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;

public class InternetSpeedTest {

    public static void main(String[] args) {
        try {
            // Define a URL for a test server
            URL url = new URL("http://www.google.com"); // Replace with your desired test server

            // Open a connection to the server
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.connect();

            // Get response time
            long startTime = System.currentTimeMillis();
            connection.getInputStream();
            long endTime = System.currentTimeMillis();
            long responseTime = endTime - startTime;

            // Calculate download speed (approximate)
            // Assuming a small file size for simplicity
            int fileSizeInBytes = 1024; // Example: 1KB
            double downloadSpeedMbps = (fileSizeInBytes / (double) responseTime) * 8 / 1024 / 1024; 

            System.out.println("Download Speed (Approx.): " + downloadSpeedMbps + " Mbps");

        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}