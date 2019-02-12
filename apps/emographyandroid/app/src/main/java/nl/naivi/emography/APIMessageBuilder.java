package nl.naivi.emography;

import android.util.Log;
import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;

/**
 * Manages multiple NAP API messages.
 * A collection of NAP API messages can be send as a list of commands to a NAP application.
 */
class APIMessageBuilder
{
    private JSONArray mMessages = null;                 ///< List of constructed messages
    private JSONObject mContainer = new JSONObject();   ///< Container of the constructed messages

    /**
     * Create the builder
     */
    APIMessageBuilder() {
        clear();
    }

    /**
     * Extracts all the api messages and caches them internally.
     * This call clears all previously cached messages.
     * The result of this operation is stored in outMessages.
     * @param apiReply the json formatted string dispatched by the NAP application
     * @param outMessages contains all the extracted messages
     * @return if extracting all messages succeeded
     */
    boolean parseReply(String apiReply, ArrayList<APIMessage> outMessages) {

        outMessages.clear();
        boolean all_extracted = true;
        try
        {
            mContainer = new JSONObject(apiReply);
            mMessages  = mContainer.getJSONArray("Objects");
            for(int obj =0; obj < mMessages.length(); obj++)
            {
                APIMessage extracted_msg = APIMessage.fromReply(mMessages.getJSONObject(obj));
                if(extracted_msg != null) {
                    outMessages.add(extracted_msg);
                }
                else {
                    all_extracted = false;
                }
            }
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
            return false;
        }
        return all_extracted;
    }

    /**
     * Add a manually constructed API message
     * @param apiMessage the nap api message to add
     */
    void addMessage(APIMessage apiMessage) {
        mMessages.put(apiMessage.getJSON());
    }

    /**
     * Creates a new API message and adds it to the queue.
     * @param command action associated with the message, ie: 'updateView'
     * @return the newly created API message
     */
    APIMessage addMessage(String command) {
        APIMessage msg = new APIMessage(command);
        mMessages.put(msg.getJSON());
        return msg;
    }

    /**
     * @return all collected API messaged as a string, empty on error.
     */
    String asString() {
        String json_string = "";
        try {
            json_string = mContainer.toString(4);
        } catch (Exception e) {
            Log.e("error", json_string);
        }
        return  json_string;
    }

    /**
     * @return all collected API messaged as a string, empty on error.
     */
    @Override
    public String toString() {
        return asString();
    }

    /**
     * Clears all collected API messages and binds a new array
     * Call this before creating a new set of messages that is send over
     */
    void clear() {
        try {
            mContainer.remove("Objects");
            mMessages = new JSONArray();
            mContainer.put("Objects", mMessages);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }
}
