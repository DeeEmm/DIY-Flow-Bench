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
    /// <summary>
    /// The <see cref="Response"/> from a <see cref="GetFlowCommand"/>.
    /// </summary>
    public class GetFlowResponse : Response
    {
        internal GetFlowResponse(Command command, byte[] data) : base(command, data)
        {
            if (data.Length != 3) throw new ArgumentException($"The {GetType().Name} must contain 3 bytes.");
        }

        /// <summary>
        /// The current flow value as read by the controller in CFM.
        /// </summary>
        public int Flow { get { return Data[1]; } }
    }
}
