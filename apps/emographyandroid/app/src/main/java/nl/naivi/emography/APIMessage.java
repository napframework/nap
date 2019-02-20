package nl.naivi.emography;

import android.util.JsonReader;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Represents a JAVA NAP API message.
 * Use this class to create an api message that can be send to a running NAP application.
 * This class can also deserialize a reply coming from a running NAP application.
 * Use the APIMessageBuilder to deserialize a list of messages.
 * When sending messages to a running NAP application use the APIMessageBuilder to create messages.
 * Messages are always send and received as a bundle.
 */
class APIMessage
{
    private JSONObject mMessage = null;     ///< The constructed api message
    private JSONArray mArguments = null;     ///< The arguments of the message

    /**
     * NAP API message constructor
     * @param command the action of the message, ie: 'updateView' etc.
     */
    APIMessage(String command)
    {
        // Create message
        mMessage = new JSONObject();

        // Create arguments
        mArguments = new JSONArray();

        try {
            mMessage.put("Type", "nap::APIMessage");
            mMessage.put("mID", UUID.randomUUID().toString());
            mMessage.put("Name", command);
            mMessage.put("Arguments", mArguments);
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Private constructor, used when deserializing an api reply
     */
    private APIMessage() { }

    /**
     * Constructs a message based on a reply from the NAP environment
     * @param messageObject the message as json object, given by the api message builder.
     * @return the extracted api messages, nullptr if message extraction fails.
        */
    static APIMessage fromReply(JSONObject messageObject) {

        APIMessage new_msg = new APIMessage();
        try
        {
            new_msg.mMessage = messageObject;
            new_msg.mArguments = messageObject.getJSONArray("Arguments");
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
            return null;
        }
        return new_msg;
    }

    /**
     * @return this message as a json object
     */
    JSONObject getJSON() {
        return mMessage;
    }

    /**
     * Get the name of this message, ie: updateView or stressReply etc. Empty if not found
     * @return the name (id) of this api message.
     */
    String getName() {
        try {
            return mMessage.getString("Name");
        }
        catch (Exception e) {
            Log.e("error", e.getMessage());
            return "";
        }
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
     * Finds an argument with the given name in the lists of arguments
     * @param id the name of the argument to find
     * @param outError contains the error message if the argument can't be extracted
     */
    private JSONObject getArgument(String id, StringBuilder outError) {
        JSONObject found_arg = null;
        try{
            for(int i=0; i<mArguments.length(); i++) {
                JSONObject cur_arg = mArguments.getJSONObject(i);
                if (cur_arg.getString("mID").equals(id)) {
                    found_arg = cur_arg;
                    break;
                }
            }
        }
        catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
            return null;
        }

        if(found_arg == null) {
            outError.append(String.format("Unable to find argument with id: %s", id));
            Log.e("error", outError.toString());
        }

        return found_arg;
    }

    /**
     * The array as a json object, also ensures size of outgoing array matches json.
     * @param argument json object to fetch array from.
     * @param outArray user array, scaled to contain the right number of future elements.
     * @param error contains the error when operation fails, empty otherwise.
     * @return the array as a json object.
     */
    private <T> JSONArray getArray(JSONObject argument, ArrayList<T> outArray, StringBuilder error) {

        JSONArray array_obj = null;
        try {
            array_obj = argument.getJSONArray("Value");
            outArray.ensureCapacity(array_obj.length());
            return array_obj;
        } catch (Exception e) {
            error.append(e.getMessage());
            Log.e("error", e.getMessage());
            return null;
        }
    }


    /**
     * Adds a string argument to this message.
     * @param id name of the argument.
     * @param value value of the argument.
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
     * Returns the value associated with the given argument as a string.
     * @param id name of the argument
     * @param outError contains the error if operation failed, empty otherwise.
     * @return value associated with argument as string, null if not available
     */
    String getString(String id, StringBuilder outError)
    {
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return null;

        try {
            return found_arg.getString("Value");
        } catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
        return null;
    }


    /**
     * Returns the value associated with the given argument as an array of string values.
     * @param id name of the argument.
     * @param outArray array that will be populated with data.
     * @param outError contains the error if the operation failed.
     */
    void getStringArray(String id, ArrayList<String> outArray, StringBuilder outError)
    {
        outArray.clear();
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return;

        try {
            JSONArray array = found_arg.getJSONArray("Value");
            outArray.ensureCapacity(array.length());
            for(int i=0; i<array.length(); i++) {
                outArray.add(i, array.getString(i));
            }
        } catch (Exception e){
            outError.append(e.getMessage());
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
     * Returns the value associated with the given argument as an array of int values.
     * @param id name of the argument.
     * @param outArray array that will be populated with data.
     * @param outError contains the error if the operation failed.
     */
    void getIntArray(String id, ArrayList<Integer> outArray, StringBuilder outError)
    {
        outArray.clear();
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return;

        try {
            JSONArray array = found_arg.getJSONArray("Value");
            outArray.ensureCapacity(array.length());
            for(int i=0; i<array.length(); i++) {
                outArray.add(i, array.getInt(i));
            }
        } catch (Exception e){
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
    }

    /**
     * Returns the value associated with the given argument as an int.
     * @param id name of the argument
     * @param outError contains the error if operation failed, empty otherwise.
     * @return value associated with argument as int, -1 if not available
     */
    int getInt(String id, StringBuilder outError)
    {
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return -1;

        try {
            return found_arg.getInt("Value");
        } catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
        return -1;
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
     * Returns the value associated with the given argument as a long.
     * @param id name of the argument
     * @param outError contains the error if operation failed, empty otherwise.
     * @return value associated with argument as long, -1 if not available
     */
    long getLong(String id, StringBuilder outError)
    {
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return -1;

        try {
            return found_arg.getLong("Value");
        } catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
        return -1;
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
     * Returns the value associated with the given argument as a boolean.
     * @param id name of the argument
     * @param outError the error message when the argument isn't available, empty on success.
     * @return value associated with argument as bool, always false when an error occurs.
     */
    boolean getBool(String id, StringBuilder outError)
    {
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return false;

        try {
            return found_arg.getBoolean("Value");
        } catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
        return false;
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


    /**
     * Returns the value associated with the given argument as a double.
     * This function should be used when given a float or double value from a NAP environment.
     * JSON for JAVA (Android) doesn't support float values, only double.
     * @param id name of the argument
     * @param outError contains the error if operation failed, empty otherwise.
     * @return value associated with argument as long, -1.0 if not available
     */
    double getDouble(String id, StringBuilder outError)
    {
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return -1.0;

        try {
            return found_arg.getDouble("Value");
        } catch (Exception e) {
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
        return -1.0;
    }


    /**
     * Returns the value associated with the given argument as an array of double values.
     * Note that JSON in JAVA doesn't support float values, only doubles when working with arrays.
     * It is recommended when given an array of floats or doubles from NAP to use this function.
     * @param id name of the argument.
     * @param outArray array that will be populated with data.
     * @param outError contains the error if the operation failed.
     */
    void getDoubleArray(String id, ArrayList<Double> outArray, StringBuilder outError)
    {
        outArray.clear();
        JSONObject found_arg = getArgument(id, outError);
        if(found_arg == null)
            return;

        try {
            JSONArray array = found_arg.getJSONArray("Value");
            outArray.ensureCapacity(array.length());
            for(int i=0; i<array.length(); i++) {
                outArray.add(i, array.getDouble(i));
            }
        } catch (Exception e){
            outError.append(e.getMessage());
            Log.e("error", e.getMessage());
        }
    }
}