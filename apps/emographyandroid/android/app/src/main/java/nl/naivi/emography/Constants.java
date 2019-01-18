package nl.naivi.emography;

public class Constants
{
    public interface ACTION
    {
        // To the service
        String STOP_SERVICE = "nl.naivi.emography.action.stopservice";
        String REQUEST_LOG_FROM_SERVICE = "nl.naivi.emography.action.requestlog";

        // To activity
        String STOP_ACTIVITY = "nl.naivi.emography.action.stopapp";
        String LOG_TO_ACTIVITY = "nl.naivi.emography.action.logtoactivity";
    }

    public interface NOTIFICATIONS
    {
        int FOREGROUND_SERVICE_ID = 101;
        String CHANNEL_ID = "emography_service_demo";
    }
}
