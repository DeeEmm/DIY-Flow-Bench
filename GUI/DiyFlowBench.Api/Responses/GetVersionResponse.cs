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
using DiyFlowBench.Api.Commands;
using System;

namespace DiyFlowBench.Api.Responses
{
    public class GetVersionResponse : Response
    {
        internal GetVersionResponse(Command command, byte[] data) : base(command, data)
        {
            if (data.Length != 5) throw new ArgumentException("The Get Version response must be 5 bytes.");
        }

        public int Major { get { return Data[1]; } }

        public int Minor { get { return Data[2]; } }

        public int Revision { get { return Data[3]; } }
    }
}
