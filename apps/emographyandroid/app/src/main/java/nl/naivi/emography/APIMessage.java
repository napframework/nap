package nl.naivi.emography;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Represents a JAVA NAP API message.
 * Use this class to create an api message that can be send to a running NAP application.
 */
class APIMessage
{
    private JSONObject mMessage = new JSONObject();     ///< The constructed api message
    private JSONArray mArguments = new JSONArray();     ///< The arguments of the message

    /**
     * NAP API message constructor
     * @param command the action of the message, ie: 'updateView' etc.
     */
    APIMessage(String command)
    {
        try {
            mMessage.put("Type", "nap::APIMessage");
            mMessage.put("mID", command);
            mMessage.put("Arguments", mArguments);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * @return this message as a json object
     */
    JSONObject getJSON() {
        return mMessage;
    }

    /**
     * Helper method that adds a new argument (APIValue) to this message
     * @param id name of the argument
     * @param type value type of the argument
     * @return the newly created and added argument
     */
    private JSONObject addArgument(String id, String type)
    {
        JSONObject arg = new JSONObject();
        try {
            arg.put("Type", type);
            arg.put("mID", id);
            mArguments.put(arg);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
        return arg;
    }

    /**
     * Adds a string argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addString(String id, String value) {
        try {
            JSONObject arg = addArgument(id, "nap::APIString");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Adds an integer argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addInt(String id, int value) {
        try {
            JSONObject arg = addArgument(id, "nap::APIInt");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Adds a float argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addFloat(String id, float value) {
        try {
            JSONObject arg = addArgument(id, "nap::APIFloat");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Adds a long argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addLong(String id, long value) {
        try {
            JSONObject arg = addArgument(id, "nap::APILong");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Adds a boolean argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addBool(String id, boolean value) {
        try {
            JSONObject arg = addArgument(id, "nap::APIBool");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Adds a double argument to this message
     * @param id name of the argument
     * @param value value of the argument
     */
    void addDouble(String id, double value) {
        try {
            JSONObject arg = addArgument(id, "nap::APIDouble");
            arg.put("Value", value);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }
}
