package nl.naivi.emography;

public class Constants
{
    public interface ACTION
    {
        // To the service
        String STOP_SERVICE = "nl.naivi.emography.action.stopservice";
        String API_SEND_MESSAGE = "nl.naivi.emography.action.requestdata";

        // To activity
        String STOP_ACTIVITY = "nl.naivi.emography.action.stopapp";
        String API_LOG_ACTIVITY = "nl.naivi.emography.action.logtoactivity";
        String API_RESPONSE_ACTIVITY = "nl.naivi.emography.action.apiMessageActivity";
    }

    public interface NOTIFICATIONS
    {
        int FOREGROUND_SERVICE_ID = 101;
        String CHANNEL_ID = "emography_service_demo";
    }

    public interface API
    {
        String MESSAGE = "apimessage";
        String LOG = "apilog";
    }
}
