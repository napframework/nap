package nl.naivi.emography;

import android.util.Log;
import org.json.JSONArray;
import org.json.JSONObject;

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
