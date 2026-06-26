import React from "react"
import withJuceSlider, {WithJUCESliderProps} from "./withJuceSlider"
import "./styles/waveform.scss"

interface Props {
    parameterID: string
    style?: React.CSSProperties
}

const Waveform: React.FunctionComponent<Props & WithJUCESliderProps> = ({parameterID, 
    style, value, properties}) => {

    const min = properties.start
    const max = properties.end
    
    const normalize = (value: number) => Math.log(value / min) / Math.log(max / min)

    const heldSamples = Math.max(1, Math.round(Math.pow(2, normalize(value) * 7)))
    let d = ""

    for (let i = 0; i < heldSamples; i++) {
        const x0 = (i / heldSamples) * 100
        const x1 = ((i + 1) / heldSamples) * 100
        const phase = (i / heldSamples) * Math.PI * 2
        const y = Math.sin(phase)

        const svgY = (1 - (y + 1) * 0.5) * 100

        if (i === 0) {
            d += `M ${x0} ${svgY} `
        } else {
            d += `L ${x0} ${svgY} `
        }

        d += `L ${x1} ${svgY} `
    }

    return (
        <div className="waveform" style={{...style}}>
            <svg viewBox="-2 -2 104 104" preserveAspectRatio="none">
                <path d={d}/>
            </svg>
        </div>
    )
}

export default withJuceSlider(Waveform)