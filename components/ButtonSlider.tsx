import React, {useRef} from "react"
import withJuceSkewSlider, {WithJUCESkewSliderProps} from "./withJuceSkewSlider"
import "./styles/buttonslider.scss"

interface Props {
    parameterID: string
    label: string
    style?: React.CSSProperties
}

const ButtonSlider: React.FunctionComponent<Props & WithJUCESkewSliderProps> = ({label, parameterID, 
    style, value, properties, onChange, reset, dragStart, dragEnd, denormalizeValue}) => {
    const dragRef = useRef({startX: 0, startY: 0, startValue: value})

    const scaledValue = denormalizeValue(value)

    const min = properties.start
    const max = properties.end

    const handlePointerDown = (event: React.PointerEvent<HTMLDivElement>) => {
        event.preventDefault()

        dragRef.current = {startX: event.clientX, startY: event.clientY, startValue: value}

        dragStart()
        event.currentTarget.setPointerCapture(event.pointerId)

        const handlePointerMove = (event: PointerEvent) => {
            const dx = event.clientX - dragRef.current.startX
            const dy = dragRef.current.startY - event.clientY

            const sensitivity = event.shiftKey ? 20000 : 2000
            const delta = (dx + dy) / sensitivity

            const startNorm = dragRef.current.startValue
            let normalized = Math.max(0, Math.min(1, startNorm + delta))

            onChange(normalized)
        }

        const handlePointerUp = () => {
            dragEnd()
            window.removeEventListener("pointermove", handlePointerMove)
            window.removeEventListener("pointerup", handlePointerUp)
        }

        window.addEventListener("pointermove", handlePointerMove)
        window.addEventListener("pointerup", handlePointerUp)
    }

    return (
        <div className="button-slider-container" style={{...style}}>
            <div className="button-slider-label">
                {label}
            </div>
            <div className="button-slider" onPointerDown={handlePointerDown}>
                <div className="button-slider-progress">
                    <div className="button-slider-rect" 
                        style={{width: `${value * 100}%`}}/>
                </div>
                <span className="button-slider-value">
                    {Math.round(scaledValue)} Hz
                </span>
            </div>
        </div>
    )
}

export default withJuceSkewSlider(ButtonSlider)