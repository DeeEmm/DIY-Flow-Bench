/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 ***/

// Development and release version - Don't forget to update the changelog!!
using System.Collections.Generic;

namespace DiyFlowBench.Api.Test.Responses
{
    public static class ResponseHelper
    {
        public static byte[] AppendChecksum(byte[] data)
        {
            byte checksum = 0;
            for (int x = 0; x < data.Length; x++)
                checksum = (byte)((checksum + data[x]) % byte.MaxValue);

            List<byte> bytes = new List<byte>(data);
            bytes.Add(checksum);
            
            return bytes.ToArray();
        }

    }
}
